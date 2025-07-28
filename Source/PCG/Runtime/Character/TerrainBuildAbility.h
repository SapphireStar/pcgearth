#pragma once

#include "CoreMinimal.h"
#include "FactoryBuilding.h"
#include "GridSelection.h"
#include "ItemAbilityComponent.h"
#include "Data/PlayerDataComponent.h"
#include "PCG/Runtime/NewPlanet/MineSphere.h"
#include "TerrainBuildAbility.generated.h"

class UWFCGeneratorComponent;
class UCameraComponent;
class UDynamicMeshComponent;
class AWFCGenerator;

USTRUCT(BlueprintType)
struct FBuildingPreviewMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	bool bCanPlaceBuilding;

	UPROPERTY(BlueprintReadWrite)
	int RequiredWood;

	UPROPERTY(BlueprintReadWrite)
	FString Message;
};
UCLASS()
class PCG_API UTerrainBuildAbility : public UItemAbilityComponent
{
	GENERATED_BODY()

public:
	UTerrainBuildAbility();

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
	virtual bool CheckCanBuildFactory(FVector Position, int Volume, FBox GridBounds);
	float CalculateFactoryRadius(int Volume);
	void ChangeFactorySphereColor(FLinearColor NewColor);
	bool ProcessBuilding(class AGeometryPlanetActor* Planet, const FHitResult& HitResult, FBox GridBounds, FIntVector GridSize, AMineSphere* MineSphere);
	void FlattenTerrain(class AGeometryPlanetActor* Planet, const TArray<int32>& VertexIndices, FBox GridBounds);
	bool SpawnBuilding(class AGeometryPlanetActor* Planet, const FHitResult& HitResult, FBox GridBounds, FIntVector GridSize, AMineSphere* MineSphere, FVector LowestVertexPos);
	void SpawnFactoryActor(FVector Position, int Volume, AMineSphere* MineSphere, float Radius);
	bool TryConsumeWood(int& outVolume);
	
	int FindVertex(const FVector& Target, UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID);
	int FindLowestVertex(UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID);
	FRotator FindNormalOnPlanet(FVector ImpactPosition, FVector PlanetPosition);
	FIntVector CalculateWFCGridSize(FBox GridBounds);
	bool ValidateGridBounds(FBox GridBounds);
	AMineSphere* CheckIsOnMineSphere(FBox GridBounds);
	
	void SelectPlanet(AGeometryPlanetActor* Planet, FHitResult& HitResult);
	void DeselectPlanet();

protected:
	UPROPERTY()
	TObjectPtr<UWFCGeneratorComponent> WFCGeneratorComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> FactorySphereMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMesh> FactorySphereMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterial> FactorySphereMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterialInstanceDynamic> FactorySphereDynamicMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	float VertexSelectionTolerance = 500.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	float SelectRange = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor AvailableColor =  FLinearColor::Green;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor UnavailableColor =  FLinearColor::Red;

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