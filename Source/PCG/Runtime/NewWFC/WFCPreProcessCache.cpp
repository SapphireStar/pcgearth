#include "WFCPreProcessCache.h"

void UWFCPreProcessCache::CreateCacheDataTable()
{
#if WITH_EDITOR
    if (!CacheDataTable)
    {
        CacheDataTable = NewObject<UDataTable>(this);
        CacheDataTable->RowStruct = FWFCPreProcessCacheTableRow::StaticStruct();
        
        MarkPackageDirty();
    }
#endif
}

void UWFCPreProcessCache::GenerateAllCaches()
{
    if (!TileSet)
    {
        UE_LOG(LogTemp, Error, TEXT("UWFCPreProcessCache::GenerateAllCaches: Can't find TileSet"));
        return;
    }

    if (!CacheDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("UWFCPreProcessCache::GenerateAllCaches: Can't find CacheDataTable"));
        return;
    }

    CacheMap.Empty();

    int32 TotalCaches = 0;

    for (int32 X = MinGridSize; X <= MaxGridSize; X++)
    {
        for (int32 Y = MinGridSize; Y <= MaxGridSize; Y++)
        {
            int Z = GridHeight;
            FIntVector GridSize(X, Y, Z);
            FWFCPreProcessCacheData CacheData = GenerateCacheForGridSize(GridSize);
            CacheMap.Add(GridSize, CacheData);
            TotalCaches++;
        }
    }
    
    SaveCacheToDataTable();
}

FWFCPreProcessCacheData UWFCPreProcessCache::GenerateCacheForGridSize(const FIntVector& GridSize)
{
    FWFCPreProcessCacheData CacheData;
    CacheData.GridSize = GridSize;

    FWFCCore TempCore;
    FWFCConfiguration Config;
    Config.GenerationMode = EWFCGenerationMode::GroundFirst;
    Config.GridSize = GridSize;
    Config.RandomSeed = 12345;
    Config.MaxIterations = 10000;
    Config.bEnableBacktracking = false;

    if (!TempCore.Initialize(TileSet, Config))
    {
        return CacheData;
    }

    const TMap<FWFCCoordinate, FWFCCell>& InitialGrid = TempCore.GetGrid();
    
    TArray<FWFCCoordinate> BoundaryCoords;
    for (const auto& [Coord, Cell] : InitialGrid)
    {
        if (IsBoundaryCoordinate(Coord, GridSize))
        {
            BoundaryCoords.Add(Coord);
        }
    }

    for (const FWFCCoordinate& Coord : BoundaryCoords)
    {
        TempCore.CollapseCellTo(Coord, 0);
        TempCore.PropagateConstraints();
    }

    const TMap<FWFCCoordinate, FWFCCell>& ProcessedGrid = TempCore.GetGrid();
    for (const auto& [Coord, Cell] : ProcessedGrid)
    {
        CacheData.CachedGrid.Add(Coord, FWFCCachedCellData(Cell, TileSet->GetTileCount()));
    }
    return CacheData;
}

bool UWFCPreProcessCache::IsBoundaryCoordinate(const FWFCCoordinate& Coord, const FIntVector& GridSize)
{
    return Coord.X == 0 || Coord.X == GridSize.X - 1 ||
           Coord.Y == 0 || Coord.Y == GridSize.Y - 1 ||
           Coord.Z == 0 || Coord.Z == GridSize.Z - 1;
}

FName UWFCPreProcessCache::GetRowNameForGridSize(const FIntVector& GridSize)
{
    return FName(*FString::Printf(TEXT("Grid_%d_%d_%d"), GridSize.X, GridSize.Y, GridSize.Z));
}

void UWFCPreProcessCache::SaveCacheToDataTable()
{
    if (!CacheDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("UWFCPreProcessCache::SaveCacheToDataTable: Can't find CacheDataTable"));
        return;
    }

#if WITH_EDITOR
    CacheDataTable->EmptyTable();

    for (const auto& [GridSize, CacheData] : CacheMap)
    {
        FName RowName = GetRowNameForGridSize(GridSize);
        FWFCPreProcessCacheTableRow NewRow;
        
        NewRow.GridSize = CacheData.GridSize;
        NewRow.CachedGrid = CacheData.CachedGrid;
        NewRow.CachedTileInstanceCounts = CacheData.CachedTileInstanceCounts;
        NewRow.CachedCollapseHistory = CacheData.CachedCollapseHistory;
        
        CacheDataTable->AddRow(RowName, NewRow);
    }

    MarkPackageDirty();
#endif
}

bool UWFCPreProcessCache::LoadCacheFromDataTable()
{
    if (bCacheLoaded)
    {
        return true;
    }

    if (!CacheDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("UWFCPreProcessCache::LoadCacheFromDataTable: Can't find CacheDataTable"));
        return false;
    }

    CacheMap.Empty();
    TArray<FName> RowNames = CacheDataTable->GetRowNames();

    for (const FName& RowName : RowNames)
    {
        const FWFCPreProcessCacheTableRow* Row = CacheDataTable->FindRow<FWFCPreProcessCacheTableRow>(RowName, TEXT(""));
        if (!Row)
        {
            continue;
        }

        FWFCPreProcessCacheData CacheData;
        CacheData.GridSize = Row->GridSize;
        CacheData.CachedGrid = Row->CachedGrid;
        CacheData.CachedTileInstanceCounts = Row->CachedTileInstanceCounts;
        CacheData.CachedCollapseHistory = Row->CachedCollapseHistory;
        
        CacheMap.Add(Row->GridSize, CacheData);
    }

    bCacheLoaded = true;
    UE_LOG(LogTemp, Log, TEXT("WFCPreProcessCache: Loaded %d cache entries from DataTable"), CacheMap.Num());
    return true;
}

bool UWFCPreProcessCache::GetCacheForGridSize(const FIntVector& GridSize, FWFCPreProcessCacheData& OutCacheData)
{
    if (!bCacheLoaded)
    {
        LoadCacheFromDataTable();
    }

    if (const FWFCPreProcessCacheData* Found = CacheMap.Find(GridSize))
    {
        OutCacheData = *Found;
        return true;
    }

    return false;
}

void UWFCPreProcessCache::ClearCache()
{
    CacheMap.Empty();
    bCacheLoaded = false;
    
    if (CacheDataTable)
    {
#if WITH_EDITOR
        CacheDataTable->EmptyTable();
        MarkPackageDirty();
#endif
        UE_LOG(LogTemp, Log, TEXT("WFCPreProcessCache: Cache cleared"));
    }
}