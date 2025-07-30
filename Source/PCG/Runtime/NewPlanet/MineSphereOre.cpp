// Fill out your copyright notice in the Description page of Project Settings.


#include "MineSphereOre.h"


// Sets default values
AMineSphereOre::AMineSphereOre()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMineSphereOre::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMineSphereOre::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

int AMineSphereOre::TryStartOneMine(int Value, std::function<void(UPlayerDataComponent*)>& pfun)
{
	int ReturnOreValue = 0;
	if (RemainMinralCount >= Value)
	{
		RemainMinralCount -= Value;
		ReturnOreValue = Value;
	}
	else
	{
		int ReturnMineValue = RemainMinralCount;
		RemainMinralCount = 0;
		ReturnOreValue = ReturnMineValue;
	}
	pfun = [ReturnOreValue](UPlayerDataComponent* PlayerData)
	{
		PlayerData->ChangePlayerResourceValue(EFactoryResource::EFR_Ore, PlayerData->GetPlayerResourceValue(EFactoryResource::EFR_Ore) + ReturnOreValue);
	};
	return ReturnOreValue;
}

