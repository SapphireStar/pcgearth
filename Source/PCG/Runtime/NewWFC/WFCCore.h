// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"
#include "WFCTileSet.h"

struct FWFCPreProcessCacheData;
class UWFCPreProcessCache;
DECLARE_DELEGATE_TwoParams(FOnWFCStatusUpdate, FWFCCoordinate, int32);

struct FWFCCell
{
    FWFCCell() = default;
    FWFCCell(int32 TileCount) : PossibleTiles(false, TileCount) {}
    
    TBitArray<> PossibleTiles;    
    bool bCollapsed = false;      
    int32 CollapsedTileIndex = -1;  
    float Entropy = 0.0f;      
    
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

    FOnWFCStatusUpdate OnStatusUpdate;
private:
    UWFCTileSet* TileSet = nullptr;
    FWFCConfiguration Config;
    TMap<FWFCCoordinate, FWFCCell> Grid;
    FRandomStream RandomGenerator;
    
    TArray<TArray<TArray<int32>>> PropagationRules; 
    TQueue<FWFCCoordinate> PropagationQueue;
    
    TArray<TArray<FWFCChange>> ChangeHistory; 
    TArray<FWFCCoordinate> CollapseHistory; 
    
    TMap<FWFCCoordinate, TArray<int32>> PositionConstraints;
    TMap<int32, int32> TileInstanceCounts;
    TMap<FWFCCoordinate, TSet<int32>> BacktrackBlacklist;
    const TArray<FIntVector> DirectionVectors;

public:
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
    bool IsBoundaryCoordinate(const FWFCCoordinate& Coord) const;
    bool IsGroundCoordinate(const FWFCCoordinate& Coord) const;
    bool CheckDecorators(int TileIndex, const FWFCTileDefinition& Tile, const FWFCCoordinate& Coord) const;
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


    void RestoreState(int32 ToDepth);
        void BlacklistTile(const FWFCCoordinate& Coord, int32 TileIndex);
        bool IsTileBlacklisted(const FWFCCoordinate& Coord, int32 TileIndex) const;
        void ClearBlacklistForCoordinate(const FWFCCoordinate& Coord);

    //尝试使用Cache
public:
    void SetPreProcessCache(UWFCPreProcessCache* InCache);
    bool LoadPreProcessedGrid();

private:
    UPROPERTY()
    TObjectPtr<UWFCPreProcessCache> PreProcessCache;
    
    void ApplyCachedGrid(const FWFCPreProcessCacheData& CacheData);
};