// Fill out your copyright notice in the Description page of Project Settings.


#include "Planet.h"

#include "K2Node_SpawnActorFromClass.h"
#include "TerrainFace.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
APlanet::APlanet()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	InitializeFaces();
}

// Called when the game starts or when spawned
void APlanet::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlanet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlanet::InitializeFaces()
{
	if (!GetWorld()) return;
	if (TerrainFaces.Num()>=6)return;
	TArray<FVector> Directions = {FVector::UpVector, FVector::DownVector, FVector::LeftVector, FVector::RightVector,FVector::ForwardVector, FVector::BackwardVector};
	for (int i = 0; i < 6; i++)
	{
		TObjectPtr<ATerrainFace> terrainObj = Cast<ATerrainFace>(GetWorld()->SpawnActor(ATerrainFace::StaticClass()));
		terrainObj->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		terrainObj->Localup = Directions[i];
		terrainObj->Resolution = Resolution;
		TerrainFaces.Add(terrainObj);
	}
}

