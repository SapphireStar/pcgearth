// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataTypes.generated.h"

UENUM(BlueprintType)
enum class EDataType : uint8
{
	PlayerStatus = 0 UMETA(DisplayName =  "PlayerStatus"),
	WoodValue = 1 UMETA(DisplayName =  "WoodValue"),
	MineValue = 2 UMETA(DisplayName =  "MineValue"),
};

USTRUCT(BlueprintType, Blueprintable)
struct PCG_API FResourceStatusNew
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Value;
};

USTRUCT(BlueprintType, Blueprintable)
struct PCG_API FPlayerStatusNew
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FResourceStatusNew Wood;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FResourceStatusNew Mine;
};