// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilityControlGroup.h"
#include "DataTypes.h"
#include "Engine/DataAsset.h"
#include "AbilityControlMap.generated.h"


UCLASS(BlueprintType)
class PCG_API UAbilityControlMap : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	FAbilityControlInfo GetControlGroup(EAbilityType eAbilityType);
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAbilityControlInfo> ControlMaps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAbilityControlInfo DefaultControlGroup;
};
