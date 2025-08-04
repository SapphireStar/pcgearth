// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseBuilding.h"
#include "FactoryBuilding.h"
#include "FactoryCrafter.generated.h"

UCLASS()
class PCG_API ACraftingBuilding : public ABaseBuilding
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACraftingBuilding();
	void SetRecipeInfo(FFactoryRecipeInfo RecipeInfo);
protected:
	virtual bool StartOneProduce() override;
	bool CheckCanProduce();

public:
	virtual FTooltipInfo GetFactoryTooltipInfo_Implementation() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFactoryRecipeInfo RecipeInfo;

	TArray<int> LastInputValues;
	int LastOutputValue;
};
