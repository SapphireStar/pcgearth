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

UENUM(BlueprintType)
enum class EFactoryResource : uint8
{
	EFR_Wood = 0		UMETA(DisplayName = "Wood"),
	EFR_Stone 			UMETA(DisplayName = "Stone"),
	EFR_Ore 			UMETA(DisplayName = "Ore"),
	EFR_Metal 			UMETA(DisplayName = "Metal"),
};

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	None,
	TerrainBuild,
	TerrainDig,
	GetResource,
	TestWFC,
};

USTRUCT(BlueprintType, Blueprintable)
struct PCG_API FResourceStatusNew
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFactoryResource ResourceType;

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
	FResourceStatusNew Stone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FResourceStatusNew Ore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FResourceStatusNew Metal;
};

USTRUCT(BlueprintType)
struct FResourceCost
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFactoryResource ResourceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount = 0;
};

USTRUCT(BlueprintType)
struct FFactoryRecipeInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FResourceCost> Input;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FResourceCost Output;
};

USTRUCT(BlueprintType)
struct FFactoryInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MiningCD = 5;

	// 用于调整工厂大小对工厂效率的影响
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int EfficiencyDivider = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* SphereStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterial* SphereMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FactoryRadius;
};

USTRUCT(BlueprintType, Blueprintable)
struct PCG_API FSystemStatus
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int RemainDays;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CurrentTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int TargetMineCount;
};

USTRUCT(BlueprintType, Blueprintable)
struct PCG_API FPlayerDataContainer
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPlayerStatusNew PlayerStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSystemStatus SystemStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFactoryInfo FactoryInfo;
};



