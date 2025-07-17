// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MineSphere.generated.h"

class USphereComponent;

UCLASS()
class PCG_API AMineSphere : public AActor
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

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* Sphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius;
};
