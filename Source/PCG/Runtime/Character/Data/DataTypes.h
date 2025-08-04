// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilityControlGroup.h"
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
	EFR_Gem				UMETA(DisplayName = "Gem"),
	EFR_None 			UMETA(DisplayName = "None"),
};

UENUM(BlueprintType)
enum class EPlayerAbilityPropertyType : uint8
{
	EPAPT_Damage				UMETA(DisplayName = "Damage"),
	EPAPT_DamageMultiplier		UMETA(DisplayName = "DamageMultiplier"),
	EPAPT_ResourceMultiplier	UMETA(DisplayName = "ResourceMultiplier"),
	EPAPT_SpeedMultiplier		UMETA(DisplayName = "SpeedMultiplier"),
	EPAPT_FuelMax				UMETA(DisplayName = "FuelMax"),
	EPAPT_CurrentFuel			UMETA(DisplayName = "CurrentFuel"),
};

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	None				UMETA(DisplayName = "None"),
	TerrainBuild		UMETA(DisplayName = "TerrainBuild"),
	TerrainDig			UMETA(DisplayName = "TerrainDig"),
	GetResource			UMETA(DisplayName = "GetResource"),
	TestWFC				UMETA(DisplayName = "TestWFC"),
	TerrainBuildCrafter	UMETA(DisplayName = "TerrainBuildCrafter"),
};

UENUM(BlueprintType)
enum class ERecipeType : uint8
{
	Metal		UMETA(DisplayName = "Metal"),
	Gem			UMETA(DisplayName = "Gem"),
};

UENUM(BlueprintType)
enum class EPopupType : uint8
{
	SubmitPopup		UMETA(DisplayName = "SubmitPopup"),
	GameWindowPopup	UMETA(DisplayName = "GameWindowPopup"),
};

UENUM(BlueprintType)
enum class EGameOverType : uint8
{
	EGOT_Win		UMETA(DisplayName = "Win"),
	EGOT_FuelEmpty	UMETA(DisplayName = "FuelEmpty"),
};

USTRUCT(BlueprintType, Blueprintable)
struct PCG_API FResourceInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFactoryResource ResourceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;
};

USTRUCT(BlueprintType, Blueprintable)
struct PCG_API FAbilityInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAbilityType AbilityType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EFactoryResource> CanProduceResourceType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText AbilityName;
};

USTRUCT(BlueprintType, Blueprintable)
struct PCG_API FResourceStatus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFactoryResource ResourceType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Value;
};

USTRUCT(BlueprintType, Blueprintable)
struct FPlayerAbilityInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ResourceMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentFuel;
};

USTRUCT(BlueprintType, Blueprintable)
struct PCG_API FPlayerAbilityPropertyUpgradeItem
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPlayerAbilityPropertyType PropertyType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IncreaseValue;
};
USTRUCT(BlueprintType, Blueprintable)
struct PCG_API FPlayerAbilityPropertyUpgradeRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPlayerAbilityPropertyUpgradeItem> PropertyItem;
};


USTRUCT(BlueprintType, Blueprintable)
struct PCG_API FPlayerStatusNew
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FResourceStatus> Resources;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPlayerAbilityInfo PlayerAbilityInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int PlayerCurrentRecipeIndex;
};

USTRUCT(BlueprintType, Blueprintable)
struct FFactoryRecipeInfo
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERecipeType RecipeType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FResourceStatus> Input;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FResourceStatus Output;
};

USTRUCT(BlueprintType, Blueprintable)
struct PCG_API FRequiredResourceInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FResourceStatus> RequiredResource;
};

USTRUCT(BlueprintType)
struct FFactoryInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProduceCD = 5;

	//用于调整工厂大小对工厂效率的影响
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int EfficiencyDivider = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* SphereStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterial* SphereMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FactoryRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableFactorySphereCollision;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CurrentTempleStage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFactoryResource CurrentProduceType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFactoryRecipeInfo CurrentRecipeInfo;
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
	FFactoryInfo MiningFactoryInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFactoryInfo CraftingFactoryInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FFactoryRecipeInfo> RecipeInfos;
};

USTRUCT(BlueprintType, Blueprintable)
struct PCG_API FAbilityControlInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAbilityType AbilityType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UAbilityControlGroup>> AbilityControls;
};

USTRUCT(BlueprintType, Blueprintable)
struct PCG_API FTooltipInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FResourceStatus> Input;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FResourceStatus> Consume;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FResourceStatus> Output;
	
};

/*USTRUCT(BlueprintType, Blueprintable)
struct PCG_API FGridDisplayInfo
{
	GENERATED_BODY()
};*/