// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeometryActors/GeneratedDynamicMeshActor.h"
#include "GeometryPlanet.generated.h"

UCLASS()
class PCG_API AGeometryPlanet : public AGeneratedDynamicMeshActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGeometryPlanet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	
	virtual void RebuildGeneratedMesh(UDynamicMesh* TargetMesh) override;
	UFUNCTION(BlueprintCallable)
	void MarkPlanetRefresh(bool bImmediate = false, bool bImmediateEventFrozen = false);
	
	UFUNCTION(BlueprintImplementableEvent)
	void GeneratePlanet(UDynamicMesh* TargetMesh);
	
	UFUNCTION(BlueprintCallable)
	void SelectVertices();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* PlanetSphere;

};
