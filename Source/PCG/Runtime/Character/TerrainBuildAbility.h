// TerrainBuildAbility.h
#pragma once

#include "CoreMinimal.h"
#include "ItemAbilityComponent.h"
#include "TerrainBuildAbility.generated.h"

class UWFCGeneratorComponent;
class UCameraComponent;
class UDynamicMeshComponent;
class AWFCGenerator;

UCLASS()
class PCG_API UTerrainBuildAbility : public UItemAbilityComponent
{
	GENERATED_BODY()

public:
	UTerrainBuildAbility();

protected:
	virtual void BeginPlay() override;

public:
	// 重写基类的生命周期函数
	virtual void OnInitializeAbility() override;
	virtual void OnActivateAbility() override;
	virtual void OnTickAbility() override;
	virtual void OnDeactivateAbility() override;
	
	virtual void OnStartUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera) override;
	virtual void OnKeepUsingAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera) override;
	virtual void OnCompleteUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera) override;


private:
	void ProcessTerrainBuild(class AGeometryPlanet* Planet, const FHitResult& HitResult);
	void FlattenTerrain(class AGeometryPlanet* Planet, const TArray<int32>& VertexIndices);
	void SpawnBuilding(class AGeometryPlanet* Planet, const FHitResult& HitResult);
	int FindVertex(const FVector& Target, UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID);
	int FindLowestVertex(UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID);

protected:
	UPROPERTY()
	TObjectPtr<UWFCGeneratorComponent> WFCGeneratorComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	float VertexSelectionTolerance = 500.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	float SelectRange = 10000.f;
};