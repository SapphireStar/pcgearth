// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseBuilding.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "PCG/Runtime/PCGGameMode.h"
#include "PCG/Runtime/NewPlanet/MineSphere.h"
#include "PCG/Runtime/Character/Data/DataTypes.h"
#include "FactoryBuilding.generated.h"


UCLASS()
class PCG_API AMiningBuilding : public ABaseBuilding
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMiningBuilding();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetMineSphere(AMineSphere* MineSphere);
	virtual bool StartOneProduce() override;
	virtual FTooltipInfo GetFactoryTooltipInfo_Implementation() override;

private:
	UPROPERTY()
	AMineSphere* MineSphere;

	int LastGetMinralCount;
};
