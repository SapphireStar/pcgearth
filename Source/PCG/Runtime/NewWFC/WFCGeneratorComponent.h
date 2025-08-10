#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WFCTypes.h"
#include "WFCTileSet.h"
#include "WFCCore.h"
#include "WFCPreProcessCache.h"
#include "WFCGeneratorComponent.generated.h"

class AWFCVisualizer;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWFCGenerationComplete, const FWFCGenerationResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWFCTileGenerated, const FWFCCoordinate&, Position, int32, TileIndex);

USTRUCT(BlueprintType)
struct FGenerationRequest
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly)
    int RequestId = 0;

    UPROPERTY(BlueprintReadOnly)
    FIntVector GridSize = FIntVector::ZeroValue;

    FGenerationRequest() = default;
    FGenerationRequest(FVector InLocation, FRotator InRotation, uint32 InRequestId, FIntVector InGridSize)
        : Location(InLocation), Rotation(InRotation), RequestId(InRequestId), GridSize(InGridSize) {}
};

UCLASS(ClassGroup=(WFC), meta=(BlueprintSpawnableComponent))
class PCG_API UWFCGeneratorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWFCGeneratorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void BeginDestroy() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Configuration")
    TObjectPtr<UWFCTileSet> TileSet;

    UPROPERTY()
    TObjectPtr<UWFCTileSet> CompleteTileSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Configuration")
    FWFCConfiguration Configuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Configuration")
    bool bAutoGenerateOnBeginPlay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Configuration")
    bool bVisualizeTiles = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Configuration")
    float CellSize = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Configuration")
    bool bUseAsyncGeneration = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Configuration")
    int GenerationActorPerFrame = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Configuration")
    int MaxQueueSize = 10;

    UPROPERTY(BlueprintAssignable, Category = "WFC Events")
    FOnWFCGenerationComplete OnGenerationComplete;

    UPROPERTY(BlueprintAssignable, Category = "WFC Events")
    FOnWFCTileGenerated OnTileGenerated;

public:
    UFUNCTION(BlueprintCallable, Category = "WFC")
    void InitializeWFCCore(const FWFCConfiguration& CustomConfig);
    
    UFUNCTION(BlueprintCallable, Category = "WFC")
    void StartGeneration();

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void StartGenerationWithCustomConfig(const FWFCConfiguration& CustomConfig);

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void StartGenerationWithCustomGridSizeAt(FVector Location, FRotator Rotation, FIntVector GridSize);

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void StartGenerationWithCustomConfigAt(FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "WFC")
    int ExecuteGenerationAsyncAt(FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void StopGeneration();

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void ClearGeneration();

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void ClearQueue();

    UFUNCTION(BlueprintCallable, Category = "WFC")
    int QueueGeneration(FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WFC")
    bool IsGenerating() const { return bIsProcessingQueue; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WFC")
    int32 GetQueueSize() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WFC")
    TArray<FGenerationRequest> GetQueuedRequests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WFC")
    FWFCGenerationResult GetLastResult() const { return LastResult; }

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void SetTileSet(UWFCTileSet* NewTileSet);

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void AddConstraint(const FWFCGenerationConstraint& Constraint);

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void ClearConstraints();
    
    UFUNCTION(BlueprintCallable, Category = "WFC")
    void SetVisualizationEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void RefreshVisualization();

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void NextCollapseStep();

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void PrevCollapseStep();

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void SetGridSize(int X, int Y);
    
//尝试使用缓存
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Configuration")
    TObjectPtr<UWFCPreProcessCache> PreProcessCache;

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void SetPreProcessCache(UWFCPreProcessCache* InCache);

protected:
    UPROPERTY(BlueprintReadOnly)
    bool bIsProcessingQueue = false;

    TArray<FGenerationRequest> PendingRequests;
    mutable FCriticalSection QueueLock;
    std::atomic<uint32> NextRequestId{1};

    TUniquePtr<FWFCCore> WFCCore;
    
    UPROPERTY()
    TMap<FWFCCoordinate, TObjectPtr<AActor>> SpawnedActors;

    UPROPERTY()
    TArray<TObjectPtr<USceneComponent>> SpawnedTileParents;

    UPROPERTY()
    TObjectPtr<USceneComponent> RootVisualization;

    TFuture<FWFCGenerationResult> GenerationFuture;

    UPROPERTY()
    FWFCGenerationResult LastResult;

    UPROPERTY()
    TArray<FWFCCoordinate> LastCollapseHistory;

    int CurCollapseHistoryStep;

private:
    void ExecuteGeneration();
    void ExecuteGenerationAsync();
    void ExecuteGenerationAt(FVector Location, FRotator Rotation);
    void ProcessNextRequest();
    void OnGenerationFinished(const FWFCGenerationResult& Result);
    void OnGenerationFinished(const FWFCGenerationResult& Result, FVector Location, FRotator Rotation);

    void CreateVisualization(const FWFCGenerationResult& Result);
    USceneComponent* CreateVisualizationAt(const FWFCGenerationResult& Result,  FVector Location, FRotator Rotation);
    USceneComponent* CreateVisualizationAtByFrame(const FWFCGenerationResult& Result,  FVector Location, FRotator Rotation);
    void ClearVisualization();
    AActor* SpawnTileActor(const FWFCCoordinate& Position, int32 TileIndex);
    FVector CoordinateToWorldPosition(const FWFCCoordinate& Coord) const;
    FVector CoordinateToLocalPosition(const FWFCCoordinate& Coord) const;

    void OnWFCStatusUpdate(FWFCCoordinate Coord, int32 Tile);
    UFUNCTION()
    void OnVisualizationComplete(AWFCVisualizer* Visualizer);
    UFUNCTION()
    void OnTimeZeroGameover(UClass* ClassType, EGameOverType eType);
    
};