// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryBuilding.h"
#include "GameFramework/Actor.h"
#include "PCG/Runtime/Character/Data/DataTypes.h"
#include "PCG/Runtime/NewPlanet/MineSphere.h"
#include "FactoryManager.generated.h"

UCLASS()
class PCG_API AFactoryManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFactoryManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnTimeZeroGameover(UClass* DataClassType);
	void BuildMiningFactoryAt(FVector Position, int Volume, AMineSphere* MineSphere, FFactoryInfo Info);
	void BuildCraftFactoryAt(FVector Position, int Volume, FFactoryInfo Info, FFactoryRecipeInfo RecipeInfo);

private:
	UPROPERTY()
	TObjectPtr<UPlayerDataComponent> PlayerData;
	
	UPROPERTY()
	FFactoryInfo MiningFactory;

	UPROPERTY()
	FFactoryInfo CraftFactory;

	UPROPERTY()
	TArray<TObjectPtr<ABaseBuilding>> SpawnedBuildings;
};
