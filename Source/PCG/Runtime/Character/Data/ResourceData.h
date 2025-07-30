// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataTypes.h"
#include "Engine/DataAsset.h"
#include "ResourceData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class PCG_API UResourceData : public UDataAsset
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	FResourceInfo GetResourceInfo(EFactoryResource ResourceType) const;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FResourceInfo> ResourceTypes;
};
