// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridSelection.h"
#include "ItemAbilityComponent.h"
#include "Components/DynamicMeshComponent.h"
#include "Data/PlayerDataComponent.h"
#include "PCG/Runtime/Factory/FactoryBuilding.h"
#include "PCG/Runtime/NewPlanet/MineSphere.h"
#include "TestWFCAbility.generated.h"


class UWFCGeneratorComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PCG_API UTestWFCAbility : public UItemAbilityComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTestWFCAbility();

protected:
	virtual void BeginPlay() override;

public:
	virtual void OnInitializeAbility() override;
	virtual void OnActivateAbility() override;
	virtual void OnTickAbility() override;
	virtual void OnDeactivateAbility() override;
	
	virtual void OnStartUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera) override;
	virtual void OnKeepUsingAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera) override;
	virtual void OnCompleteUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera) override;


private:
	bool ProcessTerrainBuild(class AGeometryPlanetActor* Planet, const FHitResult& HitResult, FBox GridBounds, AMineSphere* MineSphere);
	void FlattenTerrain(class AGeometryPlanetActor* Planet, const TArray<int32>& VertexIndices, FBox GridBounds);
	bool SpawnBuilding(class AGeometryPlanetActor* Planet, const FHitResult& HitResult, FBox GridBounds, AMineSphere* MineSphere);
	void SpawnFactoryActor(FVector Position, int Volume, AMineSphere* MineSphere);
	bool TryConsumeWood(int& outVolume);
	
	int FindVertex(const FVector& Target, UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID);
	int FindLowestVertex(UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID);
	FRotator FindNormalOnPlanet(FVector ImpactPosition, FVector PlanetPosition);
	void CalculateWFCGridSize(FBox GridBounds);
	bool ValidateGridBounds(FBox GridBounds);
	AMineSphere* CheckIsOnMineSphere(FBox GridBounds);
	
	void SelectPlanet(AGeometryPlanetActor* Planet, FHitResult& HitResult);
	void DeselectPlanet();

protected:
	UPROPERTY()
	TObjectPtr<UWFCGeneratorComponent> WFCGeneratorComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	float VertexSelectionTolerance = 500.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	float SelectRange = 10000.f;

private:
	UPROPERTY()
	TObjectPtr<UPlayerDataComponent> PlayerData;

	UPROPERTY()
	TObjectPtr<AGridSelectionManager> GridSelection;

	UPROPERTY()
	TObjectPtr<AGeometryPlanetActor> Planet;

	UPROPERTY()
	TArray<TObjectPtr<AMiningBuilding>> SpawnedFactories;

	FHitResult LastHitResult;

	bool bIsGridSlectionStarted = false;

};
