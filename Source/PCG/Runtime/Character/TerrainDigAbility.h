#pragma once

#include "CoreMinimal.h"
#include "ItemAbilityComponent.h"
#include "TerrainDigAbility.generated.h"

class UCameraComponent;
class UDynamicMeshComponent;

struct FTerrainAnalysis
{
	float AccumulatedDifference;
	float AverageHeight;
	float MaxHeightDiff;
	int32 LowestVertexID;
	bool bShouldFlatten;
};

UCLASS()
class PCG_API UTerrainDigAbility : public UItemAbilityComponent
{
	GENERATED_BODY()

public:
	UTerrainDigAbility();

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
	void ProcessTerrainDig(class AGeometryPlanetActor* Planet, const FHitResult& HitResult);
	void DigTerrain(class AGeometryPlanetActor* Planet, const TArray<int32>& VertexIndices, bool bForceAdaptive);
	int FindLowestVertex(UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID);
	FTerrainAnalysis AnalyzeTerrainVariation(UDynamicMeshComponent* Mesh, const TArray<int32>& VertexIndices);
	float CalculateAdaptiveDigDepth(const FTerrainAnalysis& Analysis);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging")
	float VertexSelectionTolerance = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging")
	float SelectRange = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging")
	float DigDepth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging")
	float HeightVariationThreshold = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging") 
	float MaxDigDepth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging")
	float MinDigDepth = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging")
	bool bUseAdaptiveDigging = true; 
	
};
