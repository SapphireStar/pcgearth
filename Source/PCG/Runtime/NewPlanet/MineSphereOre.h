// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MineSphere.h"
#include "PCG/Runtime/Character/CollectableItemInterface.h"
#include "MineSphereOre.generated.h"

UCLASS()
class PCG_API AMineSphereOre : public AMineSphere, public ICollectableItemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMineSphereOre();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual int TryStartOneMine(int Value, std::function<void(UPlayerDataComponent*)>& pfun) override;
	virtual EFactoryResource GetCollectableResourceType_Implementation() const override;
};
