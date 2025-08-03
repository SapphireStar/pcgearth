// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerDataComponent.h"


// Sets default values for this component's properties
UPlayerDataComponent::UPlayerDataComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayerDataComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void UPlayerDataComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPlayerDataComponent::InitializePlayerData(FPlayerDataContainer InitialPlayerData)
{
	PlayerStatus = InitialPlayerData.PlayerStatus;
	SystemStatus = InitialPlayerData.SystemStatus;
	OnInitialized.Broadcast(UPlayerDataComponent::StaticClass());
	OnPlayerStatusChanged.Broadcast(PlayerStatus, PlayerStatus);
	PlayerDataContainer = InitialPlayerData;
	SystemStatus.CurrentRecipeInfo = InitialPlayerData.RecipeInfos[0];
}


void UPlayerDataComponent::ChangePlayerResourceValue(EFactoryResource eResourceType, int NewValue)
{
	for (auto& Element : PlayerStatus.Resources)
	{
		if (Element.ResourceType == eResourceType)
		{
			int ResourceOld = Element.Value;
			Element.Value = NewValue;
			OnPlayerResourceChanged.Broadcast(eResourceType, ResourceOld, NewValue);
			return;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("UPlayerDataComponent::ChangePlayerResourceValue: Player has no such resource type: %s"), *UEnum::GetValueAsString(eResourceType));
}

void UPlayerDataComponent::ChangePlayerAbilityPropertyValue(EPlayerAbilityPropertyType ePropertyType, float NewValue)
{
	float OldValue = -1.0f;
	switch (ePropertyType)
	{
	case EPlayerAbilityPropertyType::EPAPT_Damage:
		OldValue = PlayerStatus.PlayerAbilityInfo.Damage;
		PlayerStatus.PlayerAbilityInfo.Damage = NewValue;
		OnAbilityPropertyChanged.Broadcast(ePropertyType, OldValue, NewValue);
		return;
		break;
	case EPlayerAbilityPropertyType::EPAPT_DamageMultiplier:
		OldValue = PlayerStatus.PlayerAbilityInfo.DamageMultiplier;
		PlayerStatus.PlayerAbilityInfo.DamageMultiplier = NewValue;
		OnAbilityPropertyChanged.Broadcast(ePropertyType, OldValue, NewValue);
		return;
		break;
	case EPlayerAbilityPropertyType::EPAPT_ResourceMultiplier:
		OldValue = PlayerStatus.PlayerAbilityInfo.ResourceMultiplier;
		PlayerStatus.PlayerAbilityInfo.ResourceMultiplier = NewValue;
		OnAbilityPropertyChanged.Broadcast(ePropertyType, OldValue, NewValue);
		return;
		break;
	case EPlayerAbilityPropertyType::EPAPT_SpeedMultiplier:
		OldValue = PlayerStatus.PlayerAbilityInfo.SpeedMultiplier;
		PlayerStatus.PlayerAbilityInfo.SpeedMultiplier = NewValue;
		OnAbilityPropertyChanged.Broadcast(ePropertyType, OldValue, NewValue);
		return;
		break;
	case EPlayerAbilityPropertyType::EPAPT_FuelMax:
		OldValue = PlayerStatus.PlayerAbilityInfo.FuelMax;
		PlayerStatus.PlayerAbilityInfo.FuelMax = NewValue;
		OnAbilityPropertyChanged.Broadcast(ePropertyType, OldValue, NewValue);
		return;
		break;
	case EPlayerAbilityPropertyType::EPAPT_CurrentFuel:
		OldValue = PlayerStatus.PlayerAbilityInfo.CurrentFuel;
		PlayerStatus.PlayerAbilityInfo.CurrentFuel = NewValue;
		OnAbilityPropertyChanged.Broadcast(ePropertyType, OldValue, NewValue);
		return;
		break;
	}

	UE_LOG(LogTemp, Error, TEXT("UPlayerDataComponent::ChangePlayerAbilityPropertyValue: Haven't add define for enum %s"), *UEnum::GetValueAsString(ePropertyType));
	return;
}

void UPlayerDataComponent::ChangePlayerRecipeIndex(int NewIndex)
{
	int OldIndex = PlayerStatus.PlayerCurrentRecipeIndex;
	PlayerStatus.PlayerCurrentRecipeIndex = NewIndex % PlayerDataContainer.RecipeInfos.Num();
	OnPlayerRecipeIndexChanged.Broadcast(OldIndex,  NewIndex);
}

void UPlayerDataComponent::ChangePlayerRemainDaysValue(int NewValue)
{
	int RemainDaysOld = SystemStatus.RemainDays;
	SystemStatus.RemainDays = NewValue;
	OnRemainDaysChanged.Broadcast(RemainDaysOld, NewValue);
}

void UPlayerDataComponent::ChangePlayerCurrentTimeValue(int NewValue)
{
	int CurrentTimeOld = SystemStatus.CurrentTime;
	SystemStatus.CurrentTime = NewValue;
	OnCurrentTimeChanged.Broadcast(CurrentTimeOld, NewValue);
}

void UPlayerDataComponent::ChangePlayerCurrentTempleStageValue(int NewValue)
{
	int CurrentTempleStageOld = SystemStatus.CurrentTempleStage;
	SystemStatus.CurrentTempleStage = NewValue;
	OnCurrentTempleStageChanged.Broadcast(CurrentTempleStageOld, NewValue);
}

void UPlayerDataComponent::ChangePlayerCurrentProduceType(EFactoryResource NewResourceType)
{
	SystemStatus.CurrentProduceType = NewResourceType;
	OnProduceTypeChanged.Broadcast(SystemStatus.CurrentProduceType);
}

void UPlayerDataComponent::ChangePlayerCurrentRecipe(FFactoryRecipeInfo NewRecipe)
{
	SystemStatus.CurrentRecipeInfo = NewRecipe;
	OnRecipeInfoChanged.Broadcast(SystemStatus.CurrentRecipeInfo);
}

FFactoryRecipeInfo UPlayerDataComponent::GetRecipeInfo(ERecipeType eType)
{
	for (auto Recipe : PlayerDataContainer.RecipeInfos)
	{
		if (Recipe.RecipeType == eType)
		{
			return Recipe;
		}
	}
	return FFactoryRecipeInfo();
}