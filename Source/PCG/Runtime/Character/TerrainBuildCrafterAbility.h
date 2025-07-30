// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TerrainBuildAbility.h"
#include "TerrainBuildCrafterAbility.generated.h"


UCLASS()
class PCG_API UTerrainBuildCrafterAbility : public UTerrainBuildAbility
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTerrainBuildCrafterAbility();
	virtual void OnTickAbility() override;
	virtual  void InitializeMineSphere() override;
	virtual void OnActivateAbility() override;
	virtual void OnCompleteUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera) override;
	virtual void SpawnFactoryActor(FVector Position, int Volume, AMineSphere* MineSphere, float Radius) override;
	virtual float CalculateFactoryRadius(int Volume) override;
	void SetFactoryRecipeInfo(FFactoryRecipeInfo RecipeInfo);
private:
	FFactoryRecipeInfo FactoryRecipeInfo;
};
