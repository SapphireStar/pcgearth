// WFCCore.h - 修正后的头文件
#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"
#include "WFCTileSet.h"

// 网格单元状态
struct FWFCCell
{
    TBitArray<> PossibleTiles;      // 可能的瓦片位掩码
    bool bCollapsed = false;        // 是否已坍缩
    int32 CollapsedTileIndex = -1;  // 坍缩后的瓦片索引
    float Entropy = 0.0f;           // 当前熵值
    
    FWFCCell() = default;
    FWFCCell(int32 TileCount) : PossibleTiles(false, TileCount) {}
    
    int32 GetPossibleTileCount() const { return PossibleTiles.CountSetBits(); }
    bool IsCollapsed() const { return bCollapsed; }
    bool CanPlace(int32 TileIndex) const { return PossibleTiles[TileIndex]; }
};

// 传播变更记录（用于回溯）
struct FWFCChange
{
    FWFCCoordinate Position;
    int32 TileIndex;
    bool bWasRemoved; // true=移除，false=添加
    
    FWFCChange(const FWFCCoordinate& Pos, int32 Tile, bool Removed)
        : Position(Pos), TileIndex(Tile), bWasRemoved(Removed) {}
};

// WFC核心算法类
class PCG_API FWFCCore
{
public:
    FWFCCore();
    ~FWFCCore();

    // 初始化
    bool Initialize(UWFCTileSet* InTileSet, const FWFCConfiguration& InConfig);
    
    // 执行生成
    FWFCGenerationResult Generate();
    
    // 重置状态
    void Reset();

    // 获取当前状态
    const TMap<FWFCCoordinate, FWFCCell>& GetGrid() const { return Grid; }
    FWFCCell* GetCell(const FWFCCoordinate& Coord);
    const FWFCCell* GetCell(const FWFCCoordinate& Coord) const;

private:
    // 配置和数据
    UWFCTileSet* TileSet = nullptr;
    FWFCConfiguration Config;
    TMap<FWFCCoordinate, FWFCCell> Grid;
    FRandomStream RandomGenerator;
    
    // 传播相关
    TArray<TArray<TArray<int32>>> PropagationRules; // [Direction][TileIndex][CompatibleTileIndex]
    TQueue<FWFCCoordinate> PropagationQueue;
    
    // 回溯相关
    TArray<TArray<FWFCChange>> ChangeHistory; // 每步的变更历史
    TArray<FWFCCoordinate> CollapseHistory;   // 坍缩历史
    
    // 约束相关
    TMap<FWFCCoordinate, TArray<int32>> PositionConstraints; // 位置特定的瓦片约束
    TMap<int32, int32> TileInstanceCounts; // 瓦片使用计数

    // 方向向量
    static const TArray<FIntVector> DirectionVectors;

private:
    // 初始化方法
    void InitializeGrid();
    void BuildPropagationRules();
    void ValidatePropagationRules();  // 添加缺失的声明
    void ApplyConstraints();
    
    // 主算法步骤
    bool RunGenerationLoop();
    FWFCCoordinate SelectNextCell();
    bool CollapseCell(const FWFCCoordinate& Coord);
    bool PropagateConstraints();
    
    // 不同的选择策略
    FWFCCoordinate SelectCellRandom();
    FWFCCoordinate SelectCellGroundFirst();
    FWFCCoordinate SelectCellLayered();
    FWFCCoordinate SelectCellCenterOut();
    
    // 传播相关
    void QueuePropagation(const FWFCCoordinate& Coord);
    bool PropagateFrom(const FWFCCoordinate& Coord);
    bool RemoveTileOption(const FWFCCoordinate& Coord, int32 TileIndex, bool bTrackChanges = true);
    
    // 回溯相关
    bool CanBacktrack() const;
    bool Backtrack();
    void SaveState();
    // void RestoreState(int32 StateIndex);  // 移除未实现的方法
    
    // 辅助方法
    bool IsValidCoordinate(const FWFCCoordinate& Coord) const;
    bool IsValidCoordinate(int X, int Y, int Z) const;
    bool IsEdgeCoordinate(const FWFCCoordinate& Coord) const;
    bool CheckCanAtEdge(const FWFCTileDefinition& Tile, const FWFCCoordinate& Coord) const;
    FWFCCoordinate GetNeighbor(const FWFCCoordinate& Coord, EWFCDirection Direction) const;
    TArray<FWFCCoordinate> GetNeighbors(const FWFCCoordinate& Coord) const;
    
    float CalculateEntropy(const FWFCCell& Cell) const;
    int32 SelectRandomTile(const FWFCCell& Cell, const FWFCCoordinate& Coord);
    
    // 约束检查
    bool CheckConstraints(const FWFCCoordinate& Coord, int32 TileIndex) const;
    bool CheckInstanceLimits(int32 TileIndex) const;
    bool CheckSupportRequirement(const FWFCCoordinate& Coord, int32 TileIndex) const;
    
    // 调试和日志
    void LogGenerationStep(const FWFCCoordinate& Coord, int32 TileIndex) const;
    void LogPropagationStep(const FWFCCoordinate& From, const FWFCCoordinate& To, int32 RemovedTile) const;
    FString GetGridStateString() const;
};