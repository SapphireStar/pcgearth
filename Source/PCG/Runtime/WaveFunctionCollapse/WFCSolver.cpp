// WFCSolver.cpp - 在现有框架基础上实现地板优先生成

#include "WFCSolver.h"

// 保持原有构造函数不变
AWFCSolver::AWFCSolver(int sizeX, int sizeY, int sizeZ, int tileNum, bool periodic,
                       const TArray<TArray<TArray<int>>>& propagator, const TArray<double>& weights)
{
    this->SizeX = sizeX;
    this->SizeY = sizeY;
    this->SizeZ = sizeZ;
    this->TileNum = tileNum;
    this->Periodic = periodic;
    this->CurrentStrategy = EWFCGenerationStrategy::Default;
    
    Propagator.SetNum(6);
    for (int d = 0; d < 6; d++)
    {
        Propagator[d].SetNum(propagator[d].Num());
        for (int t = 0; t < TileNum; t++)
        {
            for (int othert = 0; othert < propagator[d][t].Num(); othert++)
            {
                Propagator[d][t].Add(propagator[d][t][othert]);
            }
        }
    }

    Weights.SetNum(weights.Num());
    for (int i = 0; i < weights.Num(); i++)
    {
        Weights[i] = weights[i];
    }

    Init(sizeX, sizeY, sizeZ, periodic);
}

// 保持原有Init方法不变
void AWFCSolver::Init(int width, int length, int height, bool periodic)
{
    SizeX = width;
    SizeY = length;
    SizeZ = height;
    this->Periodic = periodic;

    WaveState.SetNum(SizeX * SizeY * SizeZ);
    Compatible.SetNum(WaveState.Num());
    for (int i = 0; i< WaveState.Num(); ++i)
    {
        WaveState[i].SetNum(TileNum);
        Compatible[i].SetNum(TileNum);
        for (int j = 0; j < TileNum; ++j)
        {
            Compatible[i][j].SetNum(6);
        }
    }

    Distribution.SetNum(TileNum);
    Observed.SetNum(SizeX * SizeY *  SizeZ);

    WeightLogWeights.SetNum(TileNum);
    SumOfWeights = 0;
    SumOfWeightLogWeights = 0;

    for (int t = 0; t < TileNum; ++t)
    {
        WeightLogWeights[t] = Weights[t] * FMath::Log2(Weights[t]);
        SumOfWeights += Weights[t];
        SumOfWeightLogWeights += WeightLogWeights[t];
    }

    StartingEntropy = FMath::Log2(SumOfWeights) - SumOfWeightLogWeights / SumOfWeights;

    SumsOfOnes.SetNum(SizeX * SizeY * SizeZ);
    SumsOfWeights.SetNum(SizeX * SizeY * SizeZ);
    SumsOfWeightLogWeights.SetNum(SizeX * SizeY * SizeZ);
    Entropies.SetNum(SizeX * SizeY * SizeZ);

    BannedStateStack.SetNum(WaveState.Num() * TileNum);
    StackSize = 0;
}

// 新增：带策略的运行方法
bool AWFCSolver::RunWithStrategy(int seed, int limit, TArray<int>& outResult, EWFCGenerationStrategy strategy)
{
    if (WaveState.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No wave state found"));
        return false;
    }
    
    CurrentStrategy = strategy;
    UE_LOG(LogTemp, Log, TEXT("WFC: Starting generation with strategy: %d"), (int)strategy);
    
    Clear();
    FRandomStream RandomGenerator(seed);

    for (int l = 0; l < limit; ++l)
    {
        int node = -1;
        
        // 根据策略选择下一个节点
        switch (CurrentStrategy)
        {
        case EWFCGenerationStrategy::FloorFirst:
            node = NextUnobservedNodeFloorFirst(RandomGenerator);
            break;
        case EWFCGenerationStrategy::BottomUp:
            node = NextUnobservedNodeBottomUp(RandomGenerator);
            break;
        default:
            node = NextUnobservedNode(RandomGenerator);
            break;
        }
        
        if (node >= 0)
        {
            Observe(node, RandomGenerator);
            bool success = Propagate();
            if (!success) return false;
        }
        else
        {
            outResult.SetNum(SizeX * SizeY * SizeZ);
            for (int i = 0; i < WaveState.Num(); ++i)
            {
                for (int t = 0; t < TileNum; ++t)
                {
                    if (WaveState[i][t])
                    {
                        Observed[i] = t;
                        outResult[i] = t;
                        break;
                    }
                }
            }
            UE_LOG(LogTemp, Log, TEXT("WFC: Generation completed successfully after %d iterations"), l);
            return true;
        }
    }

    outResult.SetNum(SizeX * SizeY * SizeZ);
    for (int i = 0; i < WaveState.Num(); ++i)
    {
        for (int t = 0; t < TileNum; ++t)
        {
            if (WaveState[i][t])
            {
                outResult[i] = t;
                break;
            }
        }
    }
    return true;
}

// 保持原有Run方法不变，但内部调用新方法
bool AWFCSolver::Run(int seed, int limit, TArray<int>& outResult)
{
    return RunWithStrategy(seed, limit, outResult, EWFCGenerationStrategy::Default);
}

// 新增：地板优先节点选择
int AWFCSolver::NextUnobservedNodeFloorFirst(FRandomStream& random)
{
    double minEntropy = DBL_MAX;
    int nodeMinEntropy = -1;
    bool foundFloorCandidate = false;
    
    // 第一阶段：优先选择可以放置地板的节点
    for (int i = 0; i < WaveState.Num(); ++i)
    {
        if (!Periodic && (i % SizeZ + N > SizeZ || (i/SizeZ)%SizeY + N > SizeY || ((i/SizeZ)/SizeY) % SizeX + N > SizeX))
            continue;
            
        int remainingValues = SumsOfOnes[i];
        if (remainingValues <= 1) continue; // 已观察的节点跳过
        
        // 检查这个节点是否可以放置地板
        bool canPlaceFloor = HasFloorTile(i);
        
        int x, y, z;
        IndexToCoordinate(i, x, y, z);
        
        // 如果还没找到地板候选者，或者这是一个地板候选者
        if (!foundFloorCandidate || canPlaceFloor)
        {
            double entropy = Entropies[i];
            double noise = 1E-6 * random.FRand();
            double adjustedEntropy = entropy + noise;
            
            // 地板优先：地板候选者有更高优先级
            if (canPlaceFloor && !foundFloorCandidate)
            {
                // 第一个地板候选者
                foundFloorCandidate = true;
                minEntropy = adjustedEntropy;
                nodeMinEntropy = i;
                UE_LOG(LogTemp, VeryVerbose, TEXT("WFC FloorFirst: First floor candidate at (%d, %d, %d)"), x, y, z);
            }
            else if (canPlaceFloor && foundFloorCandidate && adjustedEntropy < minEntropy)
            {
                // 更好的地板候选者
                minEntropy = adjustedEntropy;
                nodeMinEntropy = i;
                UE_LOG(LogTemp, VeryVerbose, TEXT("WFC FloorFirst: Better floor candidate at (%d, %d, %d)"), x, y, z);
            }
            else if (!foundFloorCandidate && adjustedEntropy < minEntropy)
            {
                // 还没找到地板候选者时的普通候选者
                minEntropy = adjustedEntropy;
                nodeMinEntropy = i;
                UE_LOG(LogTemp, VeryVerbose, TEXT("WFC FloorFirst: Non-floor candidate at (%d, %d, %d)"), x, y, z);
            }
        }
    }
    
    if (nodeMinEntropy >= 0)
    {
        int x, y, z;
        IndexToCoordinate(nodeMinEntropy, x, y, z);
        UE_LOG(LogTemp, Log, TEXT("WFC FloorFirst: Selected node (%d, %d, %d), is floor candidate: %s"), 
            x, y, z, foundFloorCandidate ? TEXT("Yes") : TEXT("No"));
    }
    
    return nodeMinEntropy;
}

// 新增：从底部向上生成
int AWFCSolver::NextUnobservedNodeBottomUp(FRandomStream& random)
{
    double minEntropy = DBL_MAX;
    int nodeMinEntropy = -1;
    int lowestLayer = GetLowestAvailableLayer();
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("WFC BottomUp: Targeting layer Z=%d"), lowestLayer);
    
    for (int i = 0; i < WaveState.Num(); ++i)
    {
        if (!Periodic && (i % SizeZ + N > SizeZ || (i/SizeZ)%SizeY + N > SizeY || ((i/SizeZ)/SizeY) % SizeX + N > SizeX))
            continue;
            
        int remainingValues = SumsOfOnes[i];
        if (remainingValues <= 1) continue;
        
        int x, y, z;
        IndexToCoordinate(i, x, y, z);
        
        // 只考虑当前最低层的节点
        if (z == lowestLayer)
        {
            double entropy = Entropies[i];
            double noise = 1E-6 * random.FRand();
            double adjustedEntropy = entropy + noise;
            
            if (adjustedEntropy < minEntropy)
            {
                minEntropy = adjustedEntropy;
                nodeMinEntropy = i;
            }
        }
    }
    
    if (nodeMinEntropy >= 0)
    {
        int x, y, z;
        IndexToCoordinate(nodeMinEntropy, x, y, z);
        UE_LOG(LogTemp, Log, TEXT("WFC BottomUp: Selected node (%d, %d, %d) at layer %d"), x, y, z, z);
    }
    
    return nodeMinEntropy;
}

// 保持原有的NextUnobservedNode方法
int AWFCSolver::NextUnobservedNode(FRandomStream& random)
{
    double min = DBL_MAX;
    int nodeMinEntropy = -1;
    for (int i = 0; i < WaveState.Num(); ++i)
    {
        if (!Periodic && (i % SizeZ + N > SizeZ || (i/SizeZ)%SizeY + N > SizeY || ((i/SizeZ)/SizeY) % SizeX + N > SizeX))
            continue;
        int remainingValues = SumsOfOnes[i];
        double entropy = Entropies[i];
        if (remainingValues > 1 && entropy <= min)
        {
            double noise = 1E-6 * random.FRand();
            nodeMinEntropy = i;
        }
    }
    return nodeMinEntropy;
}

// 新增：设置地板瓦片索引
void AWFCSolver::SetFloorTileIndices(const TArray<int>& floorIndices)
{
    FloorTileIndices = floorIndices;
    UE_LOG(LogTemp, Log, TEXT("WFC: Set %d floor tile indices"), floorIndices.Num());
}

// 新增：设置瓦片类型信息
void AWFCSolver::SetTileTypes(const TArray<FString>& tileTypeInfo)
{
    TileTypeNames = tileTypeInfo;
    
    // 自动检测地板瓦片
    FloorTileIndices.Empty();
    for (int i = 0; i < TileTypeNames.Num(); i++)
    {
        FString typeName = TileTypeNames[i].ToLower();
        if (typeName.Contains(TEXT("floor")) || typeName.Contains(TEXT("ground")) || 
            typeName.Contains(TEXT("地板")) || typeName.Contains(TEXT("地面")))
        {
            FloorTileIndices.Add(i);
            UE_LOG(LogTemp, Log, TEXT("WFC: Auto-detected floor tile %d: %s"), i, *TileTypeNames[i]);
        }
    }
}

// 辅助方法实现
void AWFCSolver::IndexToCoordinate(int index, int& x, int& y, int& z)
{
    z = index % SizeZ;
    y = (index / SizeZ) % SizeY;
    x = (index / SizeZ) / SizeY;
}

int AWFCSolver::CoordinateToIndex(int x, int y, int z) const
{
    return z + y * SizeZ + x * SizeY * SizeZ;
}

bool AWFCSolver::IsFloorTile(int tileIndex) const
{
    return FloorTileIndices.Contains(tileIndex);
}

bool AWFCSolver::HasFloorTile(int nodeIndex) const
{
    if (nodeIndex < 0 || nodeIndex >= WaveState.Num())
        return false;
        
    // 检查这个节点的波状态中是否包含地板瓦片
    for (int floorTileIndex : FloorTileIndices)
    {
        if (floorTileIndex < WaveState[nodeIndex].Num() && WaveState[nodeIndex][floorTileIndex])
        {
            return true;
        }
    }
    return false;
}

int AWFCSolver::GetLowestAvailableLayer() const
{
    // 找到最低的还有未观察节点的层
    for (int z = 0; z < SizeZ; z++)
    {
        for (int x = 0; x < SizeX; x++)
        {
            for (int y = 0; y < SizeY; y++)
            {
                int index = CoordinateToIndex(x, y, z);
                if (index < SumsOfOnes.Num() && SumsOfOnes[index] > 1)
                {
                    return z;
                }
            }
        }
    }
    return 0; // 默认返回最底层
}

// 保持其他原有方法不变
void AWFCSolver::Observe(int node, FRandomStream& random)
{
    TArray<bool>& w = WaveState[node];
    
    // Debug: 显示观察的节点信息
    int nodeX, nodeY, nodeZ;
    IndexToCoordinate(node, nodeX, nodeY, nodeZ);
    UE_LOG(LogTemp, Log, TEXT("WFC: Observing node (%d, %d, %d), index: %d"), nodeX, nodeY, nodeZ, node);
    
    for (int t = 0; t < TileNum; ++t)
    {
        Distribution[t] = w[t] ? Weights[t] : 0.f;
    }
    int RandomTile = RandomChooseTile(Distribution, random.FRand());
    
    // 如果是地板优先策略，记录选择的瓦片类型
    if (CurrentStrategy == EWFCGenerationStrategy::FloorFirst)
    {
        bool isFloorTile = IsFloorTile(RandomTile);
        UE_LOG(LogTemp, Log, TEXT("WFC FloorFirst: Node (%d, %d, %d) collapsed to tile %d (%s)"), 
            nodeX, nodeY, nodeZ, RandomTile, isFloorTile ? TEXT("FLOOR") : TEXT("NON-FLOOR"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("WFC: Node (%d, %d, %d) collapsed to tile %d"), nodeX, nodeY, nodeZ, RandomTile);
    }
    
    for (int t = 0; t < TileNum; ++t)
    {
        if (w[t] != (t==RandomTile))
        {
            Ban(node, t);
        }
    }
}

bool AWFCSolver::Propagate()
{
    // 方向名称，便于调试输出
    const TCHAR* DirectionNames[] = { TEXT("Up(+Z)"), TEXT("Down(-Z)"), TEXT("Left(-Y)"), TEXT("Right(+Y)"), TEXT("Forward(+X)"), TEXT("Backward(-X)") };
    
    while (StackSize > 0)
    {
        TPair<int, int> pair = BannedStateStack[StackSize-1];
        StackSize--;
        int nodeBanned = pair.Key;
        int nodeBannedTile = pair.Value;

        int nodeBannedX, nodeBannedY, nodeBannedZ;
        IndexToCoordinate(nodeBanned, nodeBannedX, nodeBannedY, nodeBannedZ);

        UE_LOG(LogTemp, VeryVerbose, TEXT("WFC: Propagating from node (%d, %d, %d), banned tile %d"), 
            nodeBannedX, nodeBannedY, nodeBannedZ, nodeBannedTile);

        for (int d = 0; d < 6; d++)
        {
            int NodeNeighbourX = nodeBannedX + dx[d];
            int NodeNeighbourY = nodeBannedY + dy[d];
            int NodeNeighbourZ = nodeBannedZ + dz[d];
            
            UE_LOG(LogTemp, VeryVerbose, TEXT("WFC: Checking direction %s, neighbor at (%d, %d, %d)"), 
                DirectionNames[d], NodeNeighbourX, NodeNeighbourY, NodeNeighbourZ);
            
            if (!Periodic && (NodeNeighbourX < 0 || NodeNeighbourY <0 || NodeNeighbourZ < 0 ||
                NodeNeighbourX + N >  SizeX || NodeNeighbourY + N > SizeY ||  NodeNeighbourZ + N > SizeZ))
            {
                UE_LOG(LogTemp, VeryVerbose, TEXT("WFC: Neighbor (%d, %d, %d) out of bounds in direction %s, skipping"), 
                    NodeNeighbourX, NodeNeighbourY, NodeNeighbourZ, DirectionNames[d]);
                continue;
            }

            if (NodeNeighbourX < 0) NodeNeighbourX += SizeX;
            else if (NodeNeighbourX >= SizeX) NodeNeighbourX -= SizeX;
            if (NodeNeighbourY < 0) NodeNeighbourY += SizeY;
            else if (NodeNeighbourY >= SizeY) NodeNeighbourY -= SizeY;
            if (NodeNeighbourZ < 0) NodeNeighbourZ += SizeZ;
            else if (NodeNeighbourZ >= SizeZ) NodeNeighbourZ -= SizeZ;

            int NodeNeighbour = NodeNeighbourZ + NodeNeighbourY * SizeZ + NodeNeighbourX * SizeY * SizeZ;
            TArray<int>& p = Propagator[d][nodeBannedTile];
            TArray<TArray<int>>& compat = Compatible[NodeNeighbour];

            UE_LOG(LogTemp, VeryVerbose, TEXT("WFC: Wrapped neighbor coordinates: (%d, %d, %d), index: %d"), 
                NodeNeighbourX, NodeNeighbourY, NodeNeighbourZ, NodeNeighbour);

            // 显示当前邻居节点的可用tile信息
            int availableTiles = 0;
            FString availableTilesList;
            for (int t = 0; t < TileNum; ++t)
            {
                if (WaveState[NodeNeighbour][t])
                {
                    availableTiles++;
                    if (!availableTilesList.IsEmpty())
                        availableTilesList += TEXT(", ");
                    availableTilesList += FString::Printf(TEXT("%d"), t);
                }
            }
            
            UE_LOG(LogTemp, VeryVerbose, TEXT("WFC: Neighbor (%d, %d, %d) in direction %s has %d available tiles: [%s]"), 
                NodeNeighbourX, NodeNeighbourY, NodeNeighbourZ, DirectionNames[d], availableTiles, *availableTilesList);

            for (int l = 0; l < p.Num(); l++)
            {
                int NodeNeighbourTile = p[l];
                TArray<int>& comp = compat[NodeNeighbourTile];

                comp[d]--;
                UE_LOG(LogTemp, VeryVerbose, TEXT("WFC: Tile %d at neighbor (%d, %d, %d) compatibility count in direction %s reduced to %d"), 
                    NodeNeighbourTile, NodeNeighbourX, NodeNeighbourY, NodeNeighbourZ, DirectionNames[d], comp[d]);
                
                if (comp[d] == 0)
                {
                    UE_LOG(LogTemp, Log, TEXT("WFC: Banning tile %d at neighbor (%d, %d, %d) due to incompatibility in direction %s"), 
                        NodeNeighbourTile, NodeNeighbourX, NodeNeighbourY, NodeNeighbourZ, DirectionNames[d]);
                    Ban(NodeNeighbour, NodeNeighbourTile);
                }
            }
        }
    }
    
    bool result = SumsOfOnes[0] > 0;
    UE_LOG(LogTemp, VeryVerbose, TEXT("WFC: Propagation finished, result: %s"), result ? TEXT("Success") : TEXT("Failed"));
    return result;
}

void AWFCSolver::Ban(int node, int tile)
{
    int nodeX, nodeY, nodeZ;
    IndexToCoordinate(node, nodeX, nodeY, nodeZ);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("WFC: Banning tile %d at node (%d, %d, %d)"), tile, nodeX, nodeY, nodeZ);
    
    WaveState[node][tile] = false;

    TArray<int>& comp = Compatible[node][tile];

    for (int d = 0; d < 6; d++)
    {
        comp[d] = 0;
    }
    BannedStateStack[StackSize] = TPair<int, int>(node, tile);
    StackSize++;

    SumsOfOnes[node] -= 1;
    SumsOfWeights[node] -= Weights[tile];
    SumsOfWeightLogWeights[node] -= WeightLogWeights[tile];

    double sum = SumsOfWeights[node];
    Entropies[node] = FMath::Log2(sum) - SumsOfWeightLogWeights[node] / sum;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("WFC: Node (%d, %d, %d) now has %d possible tiles, entropy: %f"), 
        nodeX, nodeY, nodeZ, SumsOfOnes[node], Entropies[node]);
}

void AWFCSolver::Clear()
{
    UE_LOG(LogTemp, Log, TEXT("WFC: Clearing wave state, grid size: %dx%dx%d"), SizeX, SizeY, SizeZ);
    
    for (int i = 0; i < WaveState.Num(); i++)
    {
        for (int t = 0; t < TileNum; t++)
        {
            WaveState[i][t] = true;
            for (int d = 0; d< 6; d++)
            {
                Compatible[i][t][d] = Propagator[opposite[d]][t].Num();
            }
        }

        SumsOfOnes[i] = TileNum; // 修正：应该是TileNum
        SumsOfWeights[i] = SumOfWeights;
        SumsOfWeightLogWeights[i] = SumOfWeightLogWeights;
        Entropies[i] = StartingEntropy;
        Observed[i] = -1;
    }
    
    UE_LOG(LogTemp, Log, TEXT("WFC: Wave state cleared successfully"));
}

int AWFCSolver::RandomChooseTile(const TArray<double>& weights, double rand)
{
    double sum = 0;
    for (int i = 0; i < weights.Num(); i++)
    {
        sum += weights[i];
    }
        
    double threshold = rand * sum;
    double partialSum = 0;
    for (int i = 0; i < weights.Num(); i++)
    {
        partialSum += weights[i];
        if (partialSum >= threshold) return i;
    }
    return 0;
}