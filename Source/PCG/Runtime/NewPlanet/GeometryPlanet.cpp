// Fill out your copyright notice in the Description page of Project Settings.


#include "GeometryPlanet.h"

#include "GeometryScript/MeshDeformFunctions.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "GeometryScript/MeshSubdivideFunctions.h"


// Sets default values
AGeometryPlanet::AGeometryPlanet()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PlanetSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlanetSphere"));
	PlanetSphere->AttachToComponent(DynamicMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
	PlanetSphere->SetRelativeLocation(FVector(0, 0, 0));
	PlanetSphere->SetRelativeRotation(FRotator(0, 0, 0));
}

// Called when the game starts or when spawned
void AGeometryPlanet::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGeometryPlanet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGeometryPlanet::RebuildGeneratedMesh(UDynamicMesh* TargetMesh)
{
	Super::RebuildGeneratedMesh(TargetMesh);
	GeneratePlanet(TargetMesh);
}

void AGeometryPlanet::MarkPlanetRefresh(bool bImmediate, bool bImmediateEventFrozen)
{
	MarkForMeshRebuild(bImmediate, bImmediateEventFrozen);
}

void AGeometryPlanet::SelectVertices()
{
	/*FGeometryScriptMeshSelection selection;
	UGeometryScriptLibrary_MeshSelectionFunctions::SelectMeshElementsInSphere(
		DynamicMeshComponent->GetDynamicMesh(),selection);
	selection.ConvertToMeshIndexArray()*/
}

