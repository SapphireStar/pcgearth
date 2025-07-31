// Fill out your copyright notice in the Description page of Project Settings.


#include "MineSphereStone.h"


// Sets default values
AMineSphereStone::AMineSphereStone()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMineSphereStone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMineSphereStone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

int AMineSphereStone::TryStartOneMine(int Value, std::function<void(UPlayerDataComponent*)>& pfun)
{
	int ReturnStoneValue = 0;
	if (RemainMinralCount >= Value)
	{
		RemainMinralCount -= Value;
		ReturnStoneValue = Value;
	}
	else
	{
		int ReturnMineValue = RemainMinralCount;
		RemainMinralCount = 0;
		ReturnStoneValue = ReturnMineValue;
	}
	pfun = [ReturnStoneValue](UPlayerDataComponent* PlayerData)
	{
		PlayerData->ChangePlayerResourceValue(EFactoryResource::EFR_Stone, PlayerData->GetPlayerResourceValue(EFactoryResource::EFR_Stone) + ReturnStoneValue);
	};
	return ReturnStoneValue;
}

EFactoryResource AMineSphereStone::GetCollectableResourceType_Implementation() const
{
	return EFactoryResource::EFR_Stone;
}
