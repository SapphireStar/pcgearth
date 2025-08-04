// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataTypes.h"
#include "Engine/DataAsset.h"
#include "UpgradeItemData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class PCG_API UUpgradeItemData : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	FUpgradeItemInfo GetUpgradeItemInfo(EPlayerAbilityPropertyType UpgradeItemType) const;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FUpgradeItemInfo> UpgradeItemTypes;
};
