// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"
#include "GameFramework/Actor.h"
#include "WFCVisualizer.generated.h"

class UWFCTileSet;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVisualizationComplete, AWFCVisualizer*, Visualizer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVisualizationProgress, AWFCVisualizer*, Visualizer, int32, CompletedTasks, int32, TotalTasks);


UCLASS()
class PCG_API AWFCVisualizer : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWFCVisualizer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void StartVisualization(int ActorsPerFrame, const FWFCVisualizationData& VisualizationData);
	void StopVisualization();
	void ClearVisualization();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Destroyed() override;
	
	UFUNCTION()
	void OnTimeZeroGameover(UClass* ClassType, EGameOverType eType);
	
	FOnVisualizationComplete OnVisualizationComplete;
	FOnVisualizationProgress OnVisualizationProgress;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;
	
private:
	int ActorsPerFrame = 5;
	TArray<AActor*> SpawnedActors;
	bool bIsVisualizing;
	FWFCVisualizationData VisualizationData;
	int TotalTiles;
	int CurrentTileIndex;
	bool bShowProgress = true;

	void ProcessSpawnTasks();
	AActor* SpawnTileActor(const FWFCVisualizationTile& Tile);
	void CreateSpawnTasks(const FWFCGenerationResult& GenerationResult);
	void OnVisualizationFinished();
	float GetProgress() const
	{
		if (TotalTiles <= 0)
		{
			return 1.0f;
		}
    
		return static_cast<float>(CurrentTileIndex) / static_cast<float>(TotalTiles);
	}
};
