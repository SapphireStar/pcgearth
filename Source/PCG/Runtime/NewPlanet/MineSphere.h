// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCG/Runtime/Character/CollectableItemInterface.h"
#include "PCG/Runtime/Character/Data/PlayerDataComponent.h"
#include "MineSphere.generated.h"

class AGeometryPlanet;
class USphereComponent;

UCLASS()
class PCG_API AMineSphere : public AActor, public ICollectableItemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMineSphere();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void UpdateMineSphere(float Radius);
	virtual void SetMotherWorldPlanet(AActor* planet);

	virtual int TryStartOneMine(int Value, std::function<void(UPlayerDataComponent*)>& pfun);

	UFUNCTION(BlueprintPure)
	float GetRadius() const { return Radius; }

	UFUNCTION(BlueprintPure)
	bool GetCanMine() const { return RemainMinralCount > 0; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* Sphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius;

	//矿藏量根据半径计算，该Divider用于控制矿藏量
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MineralDivider = 10000000;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int RemainMinralCount;

private:
	UPROPERTY()
	TObjectPtr<AActor> MotherWorldPlanet;

	float InitialRadius;
	int TotalMineralCount;
};
