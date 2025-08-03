// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/DataTypes.h"
#include "UObject/Interface.h"
#include "CollectableItemInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UCollectableItemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PCG_API ICollectableItemInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	EFactoryResource GetCollectableResourceType() const;
	
	virtual EFactoryResource GetCollectableResourceType_Implementation() const { return EFactoryResource::EFR_Wood;}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnResourceBeCollected(int ItemIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnGetHit(int ItemIndex, float Damage);
};
