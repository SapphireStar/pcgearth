// Fill out your copyright notice in the Description page of Project Settings.


#include "FactoryCrafter.h"


// Sets default values
ACraftingBuilding::ACraftingBuilding()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ACraftingBuilding::SetRecipeInfo(FFactoryRecipeInfo Recipeinfo)
{
	this->RecipeInfo = Recipeinfo;
}

bool ACraftingBuilding::StartOneProduce()
{
	LastInputValues.Init(0, RecipeInfo.Input.Num());
	LastOutputValue = 0;
	if (!CheckCanProduce())
	{
		return false;
	}
	
	for (int i = 0; i < FactoryEfficiency; i++)
	{
		if (!CheckCanProduce())
		{
			break;
		}
		for (int j = 0; j < RecipeInfo.Input.Num(); j++)
		{
			EFactoryResource Type = RecipeInfo.Input[j].ResourceType;
			int NewValue = PlayerData->GetPlayerResourceValue(Type) - RecipeInfo.Input[j].Value;
			PlayerData->ChangePlayerResourceValue(Type, NewValue);
			LastInputValues[j] += RecipeInfo.Input[j].Value;
		}

		EFactoryResource Type = RecipeInfo.Output.ResourceType;
		int NewValue = PlayerData->GetPlayerResourceValue(Type) + RecipeInfo.Output.Value;
		PlayerData->ChangePlayerResourceValue(Type, NewValue);
		LastOutputValue += RecipeInfo.Output.Value;
	}
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

FTooltipInfo ACraftingBuilding::GetFactoryTooltipInfo_Implementation()
{
	FTooltipInfo TooltipInfo;
	FResourceStatus ConsumeStatus;
	ConsumeStatus.ResourceType = EFactoryResource::EFR_Wood;
	ConsumeStatus.Value = Volume;
	TooltipInfo.Consume.Add(ConsumeStatus);
	
	if (LastInputValues.Num()==0) return TooltipInfo;
	for (int i = 0; i < RecipeInfo.Input.Num(); i++)
	{
		FResourceStatus Status;
		Status.ResourceType = RecipeInfo.Input[i].ResourceType;
		Status.Value = LastInputValues[i];
		TooltipInfo.Input.Add(Status);
	}
	
	FResourceStatus OutputStatus;
	OutputStatus.ResourceType = RecipeInfo.Output.ResourceType;
	OutputStatus.Value = LastOutputValue;
	TooltipInfo.Output.Add(OutputStatus);
	return TooltipInfo;
}
