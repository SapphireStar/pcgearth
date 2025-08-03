// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataTypes.h"
#include "PlayerDataComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerStatusChanged, FPlayerStatusNew, OldValue,
                                             const FPlayerStatusNew&, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWoodChanged, int, OldValue, int, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStoneChanged, int, OldValue, int, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOreChanged, int, OldValue, int, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMetalChanged, int, OldValue, int, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGemChanged, int, OldValue, int, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerResourceChanged, EFactoryResource, ResourceType, int, OldValue, int, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerAbilityPropertyChanged, EPlayerAbilityPropertyType, PropertyType, float, OldValue, float, NewValue);



DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerRecipeIndexChanged, int, OldValue, int, NewValue);




DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRemainDaysChanged, int, OldValue, int, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentTimeChanged, int, OldValue, int, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentTempleStageChanged, int, OldValue, int, NewValue);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDataComponentInitialized, UClass*, DataClassType);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeZeroGameOver, UClass*, DataClassType, EGameOverType, eGameOverType);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProduceTypeChanged, EFactoryResource, ResourceType);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecipeInfoChanged, FFactoryRecipeInfo, RecipeInfo);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PCG_API UPlayerDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerDataComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void InitializePlayerData(FPlayerDataContainer InitialPlayerData);

	UFUNCTION(BlueprintCallable)
	FPlayerDataContainer GetPlayerData() { return PlayerDataContainer; }

	UFUNCTION(BlueprintCallable)
	void ChangePlayerResourceValue(EFactoryResource eResourceType, int NewValue);

	UFUNCTION(BlueprintPure)
	int GetPlayerResourceValue(EFactoryResource eResourceType) const
	{
		for (auto Element : PlayerStatus.Resources)
		{
			if (Element.ResourceType == eResourceType)
			{
				return Element.Value;
			}
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable)
	void ChangePlayerAbilityPropertyValue(EPlayerAbilityPropertyType ePropertyType, float NewValue);

	UFUNCTION(BlueprintPure)
	float GetPlayerAbilityPropertyValue(EPlayerAbilityPropertyType ePropertyType) const
	{
		switch (ePropertyType)
		{
		case EPlayerAbilityPropertyType::EPAPT_Damage:
			return PlayerStatus.PlayerAbilityInfo.Damage;
			break;
		case EPlayerAbilityPropertyType::EPAPT_DamageMultiplier:
			return PlayerStatus.PlayerAbilityInfo.DamageMultiplier;
			break;
		case EPlayerAbilityPropertyType::EPAPT_ResourceMultiplier:
			return PlayerStatus.PlayerAbilityInfo.ResourceMultiplier;
			break;
		case EPlayerAbilityPropertyType::EPAPT_SpeedMultiplier:
			return PlayerStatus.PlayerAbilityInfo.SpeedMultiplier;
			break;
		case EPlayerAbilityPropertyType::EPAPT_FuelMax:
			return PlayerStatus.PlayerAbilityInfo.FuelMax;
			break;
		case EPlayerAbilityPropertyType::EPAPT_CurrentFuel:
			return PlayerStatus.PlayerAbilityInfo.CurrentFuel;
			break;
		}
		UE_LOG(LogTemp, Error, TEXT("UPlayerDataComponent::GetPlayerAbilityPropertyValue: Haven't add define for enum %s"), *UEnum::GetValueAsString(ePropertyType));
		return -1.0f;
	}

	UFUNCTION(BlueprintCallable)
	void ChangePlayerRecipeIndex(int NewIndex);

	UFUNCTION(BlueprintPure)
	int GetPlayerCurrentRecipeIndex() const {return PlayerStatus.PlayerCurrentRecipeIndex;}


	UFUNCTION(BlueprintCallable)
	void ChangePlayerRemainDaysValue(int NewValue);

	UFUNCTION(BlueprintPure)
	int GetPlayerRemainDaysValue() const { return SystemStatus.RemainDays; }

	UFUNCTION(BlueprintCallable)
	void ChangePlayerCurrentTimeValue(int NewValue);

	UFUNCTION(BlueprintPure)
	int GetPlayerCurrentTimeValue() const { return SystemStatus.CurrentTime; }

	UFUNCTION(BlueprintCallable)
	void ChangePlayerCurrentTempleStageValue(int NewValue);

	UFUNCTION(BlueprintPure)
	int GetPlayerCurrentTempleStageValue() const { return SystemStatus.CurrentTempleStage; }

	UFUNCTION(BlueprintCallable)
	void ChangePlayerCurrentProduceType(EFactoryResource NewResourceType);

	UFUNCTION(BlueprintPure)
	EFactoryResource GetPlayerCurrentProduceType() const { return SystemStatus.CurrentProduceType; }

	UFUNCTION(BlueprintCallable)
	void ChangePlayerCurrentRecipe(FFactoryRecipeInfo NewRecipe);

	UFUNCTION(BlueprintPure)
	FFactoryRecipeInfo GetPlayerCurrentRecipe() const { return SystemStatus.CurrentRecipeInfo; }

	UFUNCTION(BlueprintPure)
	FFactoryRecipeInfo GetRecipeInfo(ERecipeType eType);


public:
	UPROPERTY(BlueprintAssignable)
	FOnPlayerStatusChanged OnPlayerStatusChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnPlayerResourceChanged OnPlayerResourceChanged;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnPlayerAbilityPropertyChanged  OnAbilityPropertyChanged;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerRecipeIndexChanged OnPlayerRecipeIndexChanged;


	UPROPERTY(BlueprintAssignable)
	FOnRemainDaysChanged OnRemainDaysChanged;

	UPROPERTY(BlueprintAssignable)
	FOnCurrentTimeChanged OnCurrentTimeChanged;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnCurrentTempleStageChanged OnCurrentTempleStageChanged;


	UPROPERTY(BlueprintAssignable)
	FOnDataComponentInitialized OnInitialized;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnTimeZeroGameOver OnTimeZeroGameOver;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnProduceTypeChanged OnProduceTypeChanged;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnRecipeInfoChanged OnRecipeInfoChanged;

private:
	FPlayerStatusNew PlayerStatus;
	FSystemStatus SystemStatus;
	FPlayerDataContainer PlayerDataContainer;
};
