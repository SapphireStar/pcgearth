// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ColorGenerator.h"
#include "ShapeGenerator.h"
#include "TerrainFace.h"
#include "GameFramework/Actor.h"
#include "Planet.generated.h"

UENUM(BlueprintType)
enum class ERenderFaceMask : uint8
{
	RFM_All = 0 UMETA(DisplayName = "All"),
	RFM_Up = 1 UMETA(DisplayName = "Up"),
	RFM_Down = 2 UMETA(DisplayName = "Down"),
	RFM_Left = 3 UMETA(DisplayName = "Left"),
	RFM_Right = 4 UMETA(DisplayName = "Right"),
	RFM_Back = 5 UMETA(DisplayName = "Back"),
	RFM_Forward = 6 UMETA(DisplayName = "Forward"),
};
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
	void UpdateFaces();

	void GenerateColor();

	void GeneratePlanet();
	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	int Resolution = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	TObjectPtr<UMaterial> PlanetDefaultMaterial;

	UPROPERTY(editAnywhere, BlueprintReadWrite, Category = "Planet")
	ERenderFaceMask RenderFaceMask = ERenderFaceMask::RFM_All;

//Color Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
	FColorSettings ColorSettings;

//Shape Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape Settings")
	FShapeSettings ShapeSettings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape Settings")
	TObjectPtr<UShapeGenerator> ShapeGenerator;

	TSharedPtr<UColorGenerator> ColorGenerator;
private:
	UPROPERTY()
	TArray<TObjectPtr<ATerrainFace>>  TerrainFaces;
};
