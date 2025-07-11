// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShapeGenerator.h"
#include "TerrainFace.h"
#include "GameFramework/Actor.h"
#include "Planet.generated.h"

UCLASS()
class PCG_API APlanet : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APlanet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void InitializeFaces();

	void GenerateColor();

	void GeneratePlanet();


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	int Resolution = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	float Scale = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	TObjectPtr<UMaterial> PlanetDefaultMaterial;

//Color Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
	FLinearColor PlanetColor = FLinearColor::White;

//Shape Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape Settings")
	FShapeSettings ShapeSettings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape Settings")
	TObjectPtr<UShapeGenerator> ShapeGenerator;

private:
	UPROPERTY()
	TArray<TObjectPtr<ATerrainFace>>  TerrainFaces;
};
