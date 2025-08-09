// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFCTileSet.h"
#include "GameFramework/Actor.h"
#include "TileISMManager.generated.h"

UCLASS()
class PCG_API ATileISMManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATileISMManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void InitializeIsm();

	UFUNCTION(BlueprintCallable)
	void SpawnTileAt(int tile, FVector location, FRotator rotation);

	UFUNCTION(BlueprintCallable)
	void ClearAllTiles();

	UFUNCTION(BlueprintCallable)
	void OnRestartGame(UClass* DataClassType, EGameOverType eGameOverType);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> ISMComponents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UWFCTileSet> TileSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRuntimeFloatCurve CurveData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = 0.5f, ClampMax = 2.f))
	float AnimSpeed = 1.f;
	
	TDoubleLinkedList<TPair<int,int>> TileToUpdate;

	TMap<int, float> TileAnimProgress;
	
};
