// Fill out your copyright notice in the Description page of Project Settings.


#include "FactoryCrafter.h"


// Sets default values
ACraftingBuilding::ACraftingBuilding()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ACraftingBuilding::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACraftingBuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	OnTickFactory(DeltaTime);
}

bool ACraftingBuilding::CheckCanBuildFactory(FVector Position, int Volume)
{
	return true;
}

// 初始化工厂数据，使用volume来计算工厂的效率
void ACraftingBuilding::BuildFactoryAt(FVector Position, int Volume)
{
	SetActorLocation(Position);
	OnBuildFactory(Volume);
}

void ACraftingBuilding::ActivateFactory()
{
	bIsFactoryActivated = true;
}

void ACraftingBuilding::DeactivateFactory()
{
	bIsFactoryActivated = false;
}

void ACraftingBuilding::OnBuildFactory(int Volume)
{
	PCGGameMode = Cast<APCGGameMode>(GetWorld()->GetAuthGameMode());
	if (!PCGGameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("FactoryBuilding: Gamemode is not PCGGameMode"));
		return;
	}
	PlayerData = PCGGameMode->PlayerData;

	FactoryEfficiency = Volume / FactoryInfo.EfficiencyDivider;
	
	bIsFactoryActivated = true;
}

void ACraftingBuilding::OnTickFactory(float Deltatime)
{
	if (!bIsFactoryActivated) return;
	CurrentInterval +=  Deltatime;
	if (CurrentInterval >= FactoryInfo.MiningCD)
	{
		StartOneCraft();
		CurrentInterval = 0;
	}
}

void ACraftingBuilding::OnDestroyFactory()
{
	bIsFactoryActivated = false;
}

bool ACraftingBuilding::StartOneCraft()
{
return true;
}


