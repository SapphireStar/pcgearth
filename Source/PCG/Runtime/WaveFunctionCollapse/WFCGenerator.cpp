// WFCGenerator.cpp - 修改WFCGenerator以支持地板优先生成

#include "WFCGenerator.h"
#include "WFCPropagatorDataGenerator.h"

// Sets default values
AWFCGenerator::AWFCGenerator()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    
    // 默认使用地板优先策略
    GenerationStrategy = EWFCGenerationStrategy::FloorFirst;
}

// Called when the game starts or when spawned
void AWFCGenerator::BeginPlay()
{
    Super::BeginPlay();
    PropagatorDataGenerator = MakeShared<UWFCPropagatorDataGenerator>(SocketData, BaseTileData);
    PropagatorDataGenerator->GeneratePropagatorRules(Propagator);
    CompleteTileData = PropagatorDataGenerator->CompleteTileData;
    Weights.SetNum(CompleteTileData->Tiles.Num());
    
    // 设置权重
    for (int i = 0; i < CompleteTileData->Tiles.Num(); i++)
    {
        Weights[i] = CompleteTileData->Tiles[i].Weight;
    }
    
    // 自动检测和设置地板瓦片
    SetupFloorTiles();
    
    UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Initialization complete with %d tiles"), CompleteTileData->Tiles.Num());
}

// Called every frame
void AWFCGenerator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWFCGenerator::StartWFC(int SizeX, int SizeY, int SizeZ, const FVector& Position, const FRotator& Rotation, int Seed)
{
    StartWFCWithStrategy(SizeX, SizeY, SizeZ, Position, Rotation, Seed, GenerationStrategy);
}

void AWFCGenerator::StartWFCWithStrategy(int SizeX, int SizeY, int SizeZ, const FVector& Position, const FRotator& Rotation, int Seed, EWFCGenerationStrategy Strategy)
{
    UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Starting WFC generation with strategy %d"), (int)Strategy);
    
    WFCSolver = MakeShared<AWFCSolver>(SizeX, SizeY, SizeZ, Propagator[0].Num(), false, Propagator, Weights);
    
    // 设置地板瓦片信息
    WFCSolver->SetFloorTileIndices(FloorTileIndices);
    
    // 设置瓦片类型名称用于自动检测
    TArray<FString> tileNames;
    for (const auto& tile : CompleteTileData->Tiles)
    {
        tileNames.Add(tile.TileName);
    }
    WFCSolver->SetTileTypes(tileNames);
    
    auto grid = CreateWFCGrid(SizeZ, SizeY, SizeX, Position, Rotation);
    TArray<int> Result;
    
    // 使用指定策略运行WFC
    bool success = WFCSolver->RunWithStrategy(Seed > 0 ? Seed : static_cast<int>(RandomStream.FRandRange(0, 5000.f)), 
                                             SizeX * SizeY * SizeZ * 5, Result, Strategy);
    
    if (!success)
    {
        UE_LOG(LogTemp, Warning, TEXT("WFCGenerator: WFC generation failed, using partial result"));
    }
    
    // 生成结果
    for (int i = 0; i < Result.Num(); i++)
    {
        int Z = i % SizeZ;
        int Y = (i / SizeZ) % SizeY;
        int X = (i / SizeZ) / SizeY;
        
        if (Result[i] >= 0 && Result[i] < CompleteTileData->Tiles.Num())
        {
            grid->SetGridCell(Z, Y, X, CreateBlock(Result[i]));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("WFCGenerator: Invalid tile index %d at position (%d, %d, %d)"), Result[i], X, Y, Z);
        }
    }
    
    grid->SetActorRotation(Rotation);
    UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Generation completed for grid at rotation: %s"), *Rotation.ToString());
}

void AWFCGenerator::SetupFloorTiles()
{
    FloorTileIndices.Empty();
    
    if (!CompleteTileData)
    {
        UE_LOG(LogTemp, Warning, TEXT("WFCGenerator: No complete tile data available for floor setup"));
        return;
    }
    
    // 自动检测地板瓦片
    for (int i = 0; i < CompleteTileData->Tiles.Num(); i++)
    {
        const FWFCTile& tile = CompleteTileData->Tiles[i];
        FString tileName = tile.TileName.ToLower();
        
        // 检查瓦片名称是否包含地板相关关键词
        if (tileName.Contains(TEXT("floor")) || 
            tileName.Contains(TEXT("ground")) || 
            tileName.Contains(TEXT("地板")) || 
            tileName.Contains(TEXT("地面")) ||
            tileName.Contains(TEXT("base")) ||
            tileName.Contains(TEXT("foundation")))
        {
            FloorTileIndices.Add(i);
            UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Auto-detected floor tile %d: %s"), i, *tile.TileName);
        }
    }
    
    // 如果没有自动检测到地板瓦片，可以手动设置默认的
    if (FloorTileIndices.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("WFCGenerator: No floor tiles auto-detected. You may need to manually set floor tile indices."));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Setup complete with %d floor tile types"), FloorTileIndices.Num());
    }
}

void AWFCGenerator::SetFloorTileIndices(const TArray<int>& NewFloorIndices)
{
    FloorTileIndices = NewFloorIndices;
    UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Manually set %d floor tile indices"), FloorTileIndices.Num());
    
    // 验证索引有效性
    for (int index : FloorTileIndices)
    {
        if (CompleteTileData && (index < 0 || index >= CompleteTileData->Tiles.Num()))
        {
            UE_LOG(LogTemp, Warning, TEXT("WFCGenerator: Invalid floor tile index: %d"), index);
        }
    }
}

void AWFCGenerator::AddFloorTileIndex(int TileIndex)
{
    if (CompleteTileData && TileIndex >= 0 && TileIndex < CompleteTileData->Tiles.Num())
    {
        if (!FloorTileIndices.Contains(TileIndex))
        {
            FloorTileIndices.Add(TileIndex);
            UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Added floor tile index %d (%s)"), 
                TileIndex, *CompleteTileData->Tiles[TileIndex].TileName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WFCGenerator: Invalid tile index for floor tile: %d"), TileIndex);
    }
}

void AWFCGenerator::RemoveFloorTileIndex(int TileIndex)
{
    if (FloorTileIndices.Contains(TileIndex))
    {
        FloorTileIndices.Remove(TileIndex);
        UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Removed floor tile index %d"), TileIndex);
    }
}

TArray<FString> AWFCGenerator::GetFloorTileNames() const
{
    TArray<FString> floorNames;
    
    if (CompleteTileData)
    {
        for (int index : FloorTileIndices)
        {
            if (index >= 0 && index < CompleteTileData->Tiles.Num())
            {
                floorNames.Add(CompleteTileData->Tiles[index].TileName);
            }
        }
    }
    
    return floorNames;
}

AGridSystem* AWFCGenerator::CreateWFCGrid(int SizeX, int SizeY, int SizeZ, const FVector& Position,
    const FRotator& Rotation)
{
    auto grid = GetWorld()->SpawnActor<AGridSystem>();
    grid->SetActorLocation(Position);
    GridGenerator.Add(grid);
    grid->InitializeGrid(SizeX, SizeY, SizeZ);
    return grid;
}

AWFCBlock* AWFCGenerator::CreateBlock(int t)
{
    AWFCBlock* Block = GetWorld()->SpawnActor<AWFCBlock>();
    Block->Initialize(CompleteTileData->Tiles[t]);
    
    return Block;
}