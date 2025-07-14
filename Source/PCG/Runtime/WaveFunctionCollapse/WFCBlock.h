// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFCData.h"
#include "WFCSolver.h"
#include "WFCTileData.h"
#include "GameFramework/Actor.h"
#include "WFCBlock.generated.h"


UCLASS(Blueprintable)
class PCG_API AWFCBlock : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWFCBlock();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	void Initialize(FWFCTile TileData);
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> StaticMesh;
};
