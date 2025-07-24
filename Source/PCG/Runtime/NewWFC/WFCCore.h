#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"
#include "WFCTileSet.h"

struct FWFCCell
{
    TBitArray<> PossibleTiles;    
    bool bCollapsed = false;      
    int32 CollapsedTileIndex = -1;  
    float Entropy = 0.0f;      
    
    FWFCCell() = default;
    FWFCCell(int32 TileCount) : PossibleTiles(false, TileCount) {}
    
    int32 GetPossibleTileCount() const { return PossibleTiles.CountSetBits(); }
    bool IsCollapsed() const { return bCollapsed; }
    bool CanPlace(int32 TileIndex) const { return PossibleTiles[TileIndex]; }
};

struct FWFCChange
{
    FWFCCoordinate Position;
    int32 TileIndex;
    bool bWasRemoved;
    
    FWFCChange(const FWFCCoordinate& Pos, int32 Tile, bool Removed)
        : Position(Pos), TileIndex(Tile), bWasRemoved(Removed) {}
};

class PCG_API FWFCCore
{
public:
    FWFCCore();
    ~FWFCCore();

    bool Initialize(UWFCTileSet* InTileSet, const FWFCConfiguration& InConfig);
    void UpdateGrid(const FWFCConfiguration& InConfig);
    FWFCGenerationResult Generate();
    
    void Reset();

    const TMap<FWFCCoordinate, FWFCCell>& GetGrid() const { return Grid; }
    FWFCCell* GetCell(const FWFCCoordinate& Coord);
    const FWFCCell* GetCell(const FWFCCoordinate& Coord) const;
    TArray<FWFCCoordinate> GetCollapseHistory() {return CollapseHistory;}
private:
    UWFCTileSet* TileSet = nullptr;
    FWFCConfiguration Config;
    TMap<FWFCCoordinate, FWFCCell> Grid;
    FRandomStream RandomGenerator;
    
    TArray<TArray<TArray<int32>>> PropagationRules; 
    TQueue<FWFCCoordinate> PropagationQueue;
    
    TArray<TArray<FWFCChange>> ChangeHistory; 
    TArray<FWFCCoordinate> CollapseHistory; 
    
    TMap<FWFCCoordinate, TArray<int32>> PositionConstraints; // 位置特定的瓦片约束
    TMap<int32, int32> TileInstanceCounts; // 瓦片使用计数

    static const TArray<FIntVector> DirectionVectors;

private:
    void InitializeGrid();
    void BuildPropagationRules();
    void ValidatePropagationRules();
    void ApplyConstraints();
    void CellPreProcess();
    
    bool RunGenerationLoop();
    FWFCCoordinate SelectNextCell();
    bool CollapseCell(const FWFCCoordinate& Coord);
    bool CollapseCellTo(const FWFCCoordinate& Coord, int32 TileIndex);
    bool PropagateConstraints();
    
    FWFCCoordinate SelectCellRandom();
    FWFCCoordinate SelectCellGroundFirst();
    FWFCCoordinate SelectCellLayered();
    FWFCCoordinate SelectCellCenterOut();
    
    void QueuePropagation(const FWFCCoordinate& Coord);
    bool PropagateFrom(const FWFCCoordinate& Coord);
    bool RemoveTileOption(const FWFCCoordinate& Coord, int32 TileIndex, bool bTrackChanges = true);
    
    bool CanBacktrack() const;
    bool Backtrack();
    void SaveState();
    
    bool IsValidCoordinate(const FWFCCoordinate& Coord) const;
    bool IsValidCoordinate(int X, int Y, int Z) const;
    bool IsEdgeCoordinate(const FWFCCoordinate& Coord) const;
    bool IsWallCoordinate(const FWFCCoordinate& Coord) const;
    bool CheckDecorators(const FWFCTileDefinition& Tile, const FWFCCoordinate& Coord) const;
    bool CheckCanAtEdge(const FWFCTileDefinition& Tile, const FWFCCoordinate& Coord) const;
    FWFCCoordinate GetNeighbor(const FWFCCoordinate& Coord, EWFCDirection Direction) const;
    TArray<FWFCCoordinate> GetNeighbors(const FWFCCoordinate& Coord) const;
    
    float CalculateEntropy(const FWFCCell& Cell) const;
    int32 SelectRandomTile(const FWFCCell& Cell, const FWFCCoordinate& Coord);
    
    bool CheckConstraints(const FWFCCoordinate& Coord, int32 TileIndex) const;
    bool CheckInstanceLimits(int32 TileIndex) const;
    bool CheckSupportRequirement(const FWFCCoordinate& Coord, int32 TileIndex) const;
    
    void LogGenerationStep(const FWFCCoordinate& Coord, int32 TileIndex) const;
    void LogPropagationStep(const FWFCCoordinate& From, const FWFCCoordinate& To, int32 RemovedTile) const;
    FString GetGridStateString() const;
    
};