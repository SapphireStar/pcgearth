// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "WFCTypes.h"
#include "WFCTileSet.h"
#include "WFCCore.h"
#include "WFCPreProcessCache.generated.h"

USTRUCT(BlueprintType)
struct PCG_API FWFCCachedCellData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<bool> PossibleTiles;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCollapsed = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CollapsedTileIndex = -1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Entropy = 0.0f;

    FWFCCachedCellData() = default;
    
    FWFCCachedCellData(const FWFCCell& Cell, int32 TileCount)
    {
        PossibleTiles.SetNum(TileCount);
        for (int32 i = 0; i < TileCount; i++)
        {
            PossibleTiles[i] = Cell.PossibleTiles[i];
        }
        bCollapsed = Cell.bCollapsed;
        CollapsedTileIndex = Cell.CollapsedTileIndex;
        Entropy = Cell.Entropy;
    }
};

USTRUCT(BlueprintType)
struct PCG_API FWFCPreProcessCacheData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FIntVector GridSize;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FWFCCoordinate, FWFCCachedCellData> CachedGrid;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<int32, int32> CachedTileInstanceCounts;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FWFCCoordinate> CachedCollapseHistory;

    FWFCPreProcessCacheData() = default;
};

USTRUCT(BlueprintType)
struct PCG_API FWFCPreProcessCacheTableRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FIntVector GridSize;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FWFCCoordinate, FWFCCachedCellData> CachedGrid;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<int32, int32> CachedTileInstanceCounts;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FWFCCoordinate> CachedCollapseHistory;

    FWFCPreProcessCacheTableRow() = default;
};

UCLASS(BlueprintType)
class PCG_API UWFCPreProcessCache : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cache Settings")
    TObjectPtr<UWFCTileSet> TileSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cache Settings")
    TObjectPtr<UDataTable> CacheDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cache Settings")
    int32 MinGridSize = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cache Settings")
    int32 MaxGridSize = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cache Settings")
    int32 GridHeight = 7;

private:
    TMap<FIntVector, FWFCPreProcessCacheData> CacheMap;
    bool bCacheLoaded = false;

public:
    UFUNCTION(BlueprintCallable, CallInEditor)
    void GenerateAllCaches();

    UFUNCTION(BlueprintCallable, CallInEditor)
    void SaveCacheToDataTable();

    UFUNCTION(BlueprintCallable, Category = "Cache Loading")
    bool LoadCacheFromDataTable();

    UFUNCTION(BlueprintCallable, Category = "Cache Loading")
    bool GetCacheForGridSize(const FIntVector& GridSize, FWFCPreProcessCacheData& OutCacheData);

    UFUNCTION(BlueprintCallable, Category = "Cache Loading")
    void ClearCache();

    UFUNCTION(BlueprintCallable, CallInEditor)
    void CreateCacheDataTable();

private:
    FWFCPreProcessCacheData GenerateCacheForGridSize(const FIntVector& GridSize);
    bool IsBoundaryCoordinate(const FWFCCoordinate& Coord, const FIntVector& GridSize);
    FName GetRowNameForGridSize(const FIntVector& GridSize);
};