// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "PCG/Runtime/PCGGameMode.h"
#include "PCG/Runtime/Character/Data/DataTypes.h"
#include "BaseBuilding.generated.h"

UCLASS()
class PCG_API ABaseBuilding : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABaseBuilding();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void BuildFactoryAt(FVector Position, int Volume, FFactoryInfo Info);
	void ActivateFactory();
	void DeactivateFactory();

protected:
	void OnBuildFactory(int Volume);
	void OnTickFactory(float Deltatime);
	void OnDestroyFactory();
	virtual bool StartOneProduce();
	
	UFUNCTION()
	virtual void OnPlayerSwitchAbility(EAbilityType OldAbility, EAbilityType NewAbility);
	UFUNCTION()
	void OnTerrainBuildAbilityActivated(EAbilityType eType);
	UFUNCTION()
	void OnTerrainBuildAbilityDeactivated(EAbilityType eType);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFactoryInfo FactoryInfo;

protected:
	TObjectPtr<APCGGameMode> PCGGameMode;
	
	TObjectPtr<UPlayerDataComponent> PlayerData;

	TObjectPtr<USphereComponent> SphereCollision;

	TObjectPtr<UStaticMeshComponent> SphereStaticMeshComponent;

	TObjectPtr<UStaticMeshComponent> SphereStaticMesh;

	TObjectPtr<UMaterial> SphereMaterial;

	bool bIsFactoryActivated = false;

	int FactoryEfficiency = 0;

	float CurrentInterval = 0;
};
