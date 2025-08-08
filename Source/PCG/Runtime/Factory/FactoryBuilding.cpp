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

void AMiningBuilding::SetMineSphere(AMineSphere* Minesphere)
{
	this->MineSphere = Minesphere;
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
		LastGetMinralCount = MineSphere->TryStartOneMine(FactoryEfficiency, pfun);
		int NewPlayerMine = PlayerData->GetPlayerResourceValue(EFactoryResource::EFR_Stone) + LastGetMinralCount;
		pfun(PlayerData);
		return true;
	}
	else
	{
		LastGetMinralCount = 0;
		UE_LOG(LogTemp, Log, TEXT("FactoryBuilding: MineSphere is run out of Mine"));
	}
	return false;
}

FTooltipInfo AMiningBuilding::GetFactoryTooltipInfo_Implementation()
{
	FTooltipInfo TooltipInfo;
	FResourceStatus ConsumeStatus;
	ConsumeStatus.ResourceType = EFactoryResource::EFR_Wood;
	ConsumeStatus.Value = Volume;
	TooltipInfo.Consume.Add(ConsumeStatus);
	
	FResourceStatus ResourceStatus;
	ResourceStatus.ResourceType = MineSphere->GetCollectableResourceType_Implementation();
	ResourceStatus.Value = LastGetMinralCount;
	TooltipInfo.Output.Add(ResourceStatus);
	return TooltipInfo;
}


