// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCG/Runtime/NoiseFilter/INoiseFilterInterface.h"
#include "PCG/Runtime/Utils/MinMax.h"
#include "UObject/Object.h"
#include "ShapeGenerator.generated.h"

UENUM(BlueprintType)
enum class ENoiseType : uint8
{
	SGT_SimpleNoise		= 0 UMETA(DisplayName = "Simple Noise"),
	SGT_RigidNoise		= 1 UMETA(DisplayName = "Rigid Noise"),
};

USTRUCT(BlueprintType)
struct FNoiseSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Strength = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseRoughness = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Roughness = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Center = FVector(0.f, 0.f, 0.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite	)
	int NumLayers;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Persistence = .5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeightMultiplier;
};

USTRUCT(BlueprintType)
struct FNoiseLayer
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ENoiseType NoiseType = ENoiseType::SGT_SimpleNoise;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseFirstLayerAsMask = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FNoiseSettings NoiseSettings;
};


USTRUCT(BlueprintType)
struct FShapeSettings
{
	FShapeSettings();
	FShapeSettings(const FShapeSettings& Other);
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlanetRadius = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FNoiseLayer> NoiseLayers;

	bool IsValid() const{return NoiseLayers.Num() > 0;}
};

USTRUCT(BlueprintType)
struct FColorSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCurveLinearColor> ColorGradient;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstance = nullptr;
};


UCLASS()
class PCG_API UShapeGenerator : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(FShapeSettings ShapeSettings);
	FVector CalculatePointOnPlanet(FVector pointOnUnitSphere);
	float CalculateElevationOnPlanet(FVector pointOnUnitSphere);
private:
	FShapeSettings ShapeSettings;
	TArray<TSharedPtr<INoiseFilterInterface>> NoiseFilters;
public:
	TSharedPtr<MinMax> ElevationMinMax;
};
