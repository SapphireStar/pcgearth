// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridSystem.h"
#include "WFCSocketCompatibilityData.h"
#include "GameFramework/Actor.h"
#include "WFCTileData.h"
#include "WFCGenerator.generated.h"

class UWFCTileData;
class UWFCPropagatorDataGenerator;

UCLASS()
class PCG_API AWFCGenerator : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWFCGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "WaveFunction")
	void StartWFC();

	AWFCBlock* CreateBlock(int t);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int SizeX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int SizeY;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int SizeZ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int TileNum;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Periodic;
	UPROPERTY(Editanywhere, BlueprintReadWrite)
	int N = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UWFCSocketCompatibilityData> SocketData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UWFCTileData> BaseTileData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UWFCTileData> CompleteTileData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AGridSystem> GridGenerator;

	TSharedPtr<UWFCPropagatorDataGenerator> PropagatorDataGenerator;
	TSharedPtr<AWFCSolver> WFCSolver;
	TArray<TArray<TArray<int>>> Propagator;
	TArray<double> Weights;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UWFCData> WFCData;

};
