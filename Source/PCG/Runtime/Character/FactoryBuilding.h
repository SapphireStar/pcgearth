// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "PCG/Runtime/PCGGameMode.h"
#include "PCG/Runtime/NewPlanet/MineSphere.h"
#include "FactoryBuilding.generated.h"

USTRUCT(BlueprintType)
struct FFactoryInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MiningCD = 5;

	// 用于调整工厂大小对工厂效率的影响
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int EfficiencyDivider = 1;
};

UCLASS()
class PCG_API AFactoryBuilding : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFactoryBuilding();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void BuildFactoryAt(FVector Position, int Volume, AMineSphere* MineSphere);

	UFUNCTION(BlueprintCallable)
	void ActivateFactory();
	
	UFUNCTION(BlueprintCallable)
	void DeactivateFactory();

private:
	void OnBuildFactory(int Volume);
	void OnTickFactory(float Deltatime);
	void OnDestroyFactory();

	bool StartOneMining();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFactoryInfo FactoryInfo;

private:
	UPROPERTY()
	TObjectPtr<AMineSphere> MineSphere;

	TObjectPtr<APCGGameMode> PCGGameMode;
	TObjectPtr<UPlayerDataComponent> PlayerData;

	bool bIsFactoryActivated = false;

	int FactoryEfficiency = 0;

	float CurrentInterval = 0;
};
