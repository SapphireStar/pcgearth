// Fill out your copyright notice in the Description page of Project Settings.


#include "FactoryCrafter.h"


// Sets default values
ACraftingBuilding::ACraftingBuilding()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ACraftingBuilding::SetRecipeInfo(FFactoryRecipeInfo RecipeInfo)
{
	this->RecipeInfo = RecipeInfo;
}

bool ACraftingBuilding::StartOneProduce()
{
	if (!CheckCanProduce())
	{
		return false;
	}

	//减去需要的材料
	for (int i = 0; i < RecipeInfo.Input.Num(); i++)
	{
		EFactoryResource Type = RecipeInfo.Input[i].ResourceType;
		int NewValue = PlayerData->GetPlayerResourceValue(Type) - RecipeInfo.Input[i].Value;
		PlayerData->ChangePlayerResourceValue(Type, NewValue);
	}

	//添加产出材料
	EFactoryResource Type = RecipeInfo.Output.ResourceType;
	int NewValue = PlayerData->GetPlayerResourceValue(Type) + RecipeInfo.Output.Value;
	PlayerData->ChangePlayerResourceValue(Type, NewValue);

	return true;
}

bool ACraftingBuilding::CheckCanProduce()
{
	for (int i = 0; i < RecipeInfo.Input.Num(); i++)
	{
		EFactoryResource Type = RecipeInfo.Input[i].ResourceType;
		int PlayerValue = PlayerData->GetPlayerResourceValue(Type);
		if (PlayerValue < RecipeInfo.Input[i].Value)
		{
			UE_LOG(LogTemp, Warning, TEXT("ACraftingBuilding::CheckCanProduce: Player %s is not fulfilled"),
			       *UEnum::GetValueAsString(Type));
			return false;
		}
	}
	return true;
}
