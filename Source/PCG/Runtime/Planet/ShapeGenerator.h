// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ShapeGenerator.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FShapeSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlanetRadius = 100.f;
};
UCLASS()
class PCG_API UShapeGenerator : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(FShapeSettings ShapeSettings);
	FVector CalculatePointOnPlanet(FVector pointOnUnitSphere);
private:
	FShapeSettings ShapeSettings;
};
