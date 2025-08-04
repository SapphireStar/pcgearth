// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCG/Runtime/Character/Data/DataTypes.h"
#include "UObject/Interface.h"
#include "TooltipableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UTooltipableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PCG_API ITooltipableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FTooltipInfo GetFactoryTooltipInfo();
};
