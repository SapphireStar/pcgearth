// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryBuilding.h"
#include "FactoryCrafter.generated.h"

UCLASS()
class PCG_API ACraftingBuilding : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACraftingBuilding();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual bool CheckCanBuildFactory(FVector Position, int Volume);

	UFUNCTION(BlueprintCallable)
	void BuildFactoryAt(FVector Position, int Volume);

	UFUNCTION(BlueprintCallable)
	void ActivateFactory();
	
	UFUNCTION(BlueprintCallable)
	void DeactivateFactory();

protected:
	void OnBuildFactory(int Volume);
	void OnTickFactory(float Deltatime);
	void OnDestroyFactory();

	bool StartOneCraft();
	bool CheckHasResource();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFactoryInfo FactoryInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFactoryRecipeInfo RecipeInfo;

protected:
	TObjectPtr<APCGGameMode> PCGGameMode;
	
	TObjectPtr<UPlayerDataComponent> PlayerData;

	bool bIsFactoryActivated = false;

	int FactoryEfficiency = 0;

	float CurrentInterval = 0;
};
