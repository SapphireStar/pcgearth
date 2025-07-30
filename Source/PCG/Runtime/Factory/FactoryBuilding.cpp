// Fill out your copyright notice in the Description page of Project Settings.


#include "FactoryBuilding.h"

#include "Kismet/KismetSystemLibrary.h"


// Sets default values
AMiningBuilding::AMiningBuilding()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMiningBuilding::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMiningBuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	OnTickFactory(DeltaTime);
}

void AMiningBuilding::SetMineSphere(AMineSphere* MineSphere)
{
	this->MineSphere = MineSphere;
}

bool AMiningBuilding::StartOneProduce()
{
	if (MineSphere == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AMiningBuilding: MineSphere is null!"));
	}
	
	if (MineSphere->GetCanMine())
	{
		std::function<void(UPlayerDataComponent*)> pfun;
		int NewPlayerMine = PlayerData->GetPlayerResourceValue(EFactoryResource::EFR_Stone) + MineSphere->TryStartOneMine(FactoryEfficiency, pfun);
		pfun(PlayerData);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("FactoryBuilding: MineSphere is run out of Mine"));
	}
	return false;
}