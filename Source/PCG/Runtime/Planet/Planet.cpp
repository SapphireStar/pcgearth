// Fill out your copyright notice in the Description page of Project Settings.


#include "Planet.h"

#include "K2Node_SpawnActorFromClass.h"
#include "TerrainFace.h"
#include "Kismet/GameplayStatics.h"
#include "Virtualization/VirtualizationSystem.h"


// Sets default values
APlanet::APlanet()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void APlanet::BeginPlay()
{
	Super::BeginPlay();
	GeneratePlanet();
}

// Called every frame
void APlanet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlanet::InitializeFaces()
{
	if (!GetWorld()) return;
	if (TerrainFaces.Num() >= 6) return;
	ShapeGenerator = NewObject<UShapeGenerator>();
	ShapeGenerator->Initialize(ShapeSettings);
	
	TArray<FVector> Directions = {FVector::UpVector, FVector::DownVector, FVector::LeftVector, FVector::RightVector,FVector::ForwardVector, FVector::BackwardVector};
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	for (int i = 0; i < 6; i++)
	{
		FVector SpawnLocation = GetActorLocation();
		FRotator SpawnRotation = GetActorRotation();
		TObjectPtr<ATerrainFace> terrain = Cast<ATerrainFace>(GetWorld()->SpawnActor(ATerrainFace::StaticClass(), &SpawnLocation, &SpawnRotation,SpawnParams));
		//terrainObj->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		auto comp = GetComponentByClass<UPrimitiveComponent>();
		terrain->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		terrain->InitializeTerrain(ShapeGenerator, Resolution, Directions[i], Scale);
		TerrainFaces.Add(terrain);
		
		terrain->ConstructMesh();
	}
}

void APlanet::GenerateColor()
{
	for (auto Terrain : TerrainFaces)
	{
		UMaterialInstanceDynamic* PlanetDefault = UMaterialInstanceDynamic::Create(PlanetDefaultMaterial, this);
		FLinearColor debugcolor;
		PlanetDefault->GetVectorParameterValue(FName(TEXT("Color")),debugcolor);
		PlanetDefault->SetVectorParameterValue(FName(TEXT("Color")), PlanetColor);
		PlanetDefault->GetVectorParameterValue(FName(TEXT("Color")),debugcolor);
		Terrain->GetDynamicMeshComponent()->SetMaterial(0, PlanetDefault);
		Terrain->ConstructMesh();
	}
}

void APlanet::GeneratePlanet()
{
	InitializeFaces();
	GenerateColor();
}

