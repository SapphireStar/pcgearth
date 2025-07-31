// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataTypes.h"
#include "Engine/DataAsset.h"
#include "AbilityData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class PCG_API UAbilityData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	FAbilityInfo GetAbilityInfo(EAbilityType AbilityType) const;

	UFUNCTION(BlueprintCallable)
	FAbilityInfo GetAbilityCanProduceResource(EFactoryResource ResourceType);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FAbilityInfo> Abilities;
	
};
