// WFCGeneratorComponent.h - 修正后的头文件
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WFCTypes.h"
#include "WFCTileSet.h"
#include "WFCCore.h"
#include "WFCGeneratorComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWFCGenerationComplete, const FWFCGenerationResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWFCTileGenerated, const FWFCCoordinate&, Position, int32, TileIndex);

UCLASS(ClassGroup=(WFC), meta=(BlueprintSpawnableComponent))
class PCG_API UWFCGeneratorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWFCGeneratorComponent();

protected:
    virtual void BeginPlay() override;

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
    void StartGenerationWithCustomConfigAt(FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void StopGeneration();

    UFUNCTION(BlueprintCallable, Category = "WFC")
    void ClearGeneration();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WFC")
    bool IsGenerating() const { return bIsGenerating; }

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

protected:
    UPROPERTY()
    bool bIsGenerating = false;

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
    void ExecuteGenerationAsyncAt(FVector Location, FRotator Rotation);
    void ExecuteGenerationAt(FVector Location, FRotator Rotation);
    void OnGenerationFinished(const FWFCGenerationResult& Result);
    void OnGenerationFinished(const FWFCGenerationResult& Result, FVector Location, FRotator Rotation);

    void CreateVisualization(const FWFCGenerationResult& Result);
    USceneComponent* CreateVisualizationAt(const FWFCGenerationResult& Result,  FVector Location, FRotator Rotation);
    USceneComponent* CreateVisualizationAtByFrame(const FWFCGenerationResult& Result,  FVector Location, FRotator Rotation);
    void ClearVisualization();
    AActor* SpawnTileActor(const FWFCCoordinate& Position, int32 TileIndex);
    FVector CoordinateToWorldPosition(const FWFCCoordinate& Coord) const;
    FVector CoordinateToLocalPosition(const FWFCCoordinate& Coord) const;
};