// TerrainDigAbility.h
#pragma once

#include "CoreMinimal.h"
#include "ItemAbilityComponent.h"
#include "TerrainDigAbility.generated.h"

class UCameraComponent;
class UDynamicMeshComponent;

UCLASS()
class PCG_API UTerrainDigAbility : public UItemAbilityComponent
{
	GENERATED_BODY()

public:
	UTerrainDigAbility();

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
	void ProcessTerrainDig(class AGeometryPlanet* Planet, const FHitResult& HitResult);
	void DigTerrain(class AGeometryPlanet* Planet, const TArray<int32>& VertexIndices);
	int FindLowestVertex(UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging")
	float VertexSelectionTolerance = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging")
	float SelectRange = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging")
	float DigDepth = 100.f;
};
