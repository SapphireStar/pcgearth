// Fill out your copyright notice in the Description page of Project Settings.


#include "FactoryBuilding.h"



// Sets default values
AFactoryBuilding::AFactoryBuilding()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AFactoryBuilding::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFactoryBuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	OnTickFactory(DeltaTime);
}

// 初始化工厂数据，使用volume来计算工厂的效率
void AFactoryBuilding::BuildFactoryAt(FVector Position, int Volume, AMineSphere* MineSphere)
{
	SetActorLocation(Position);
	this->MineSphere = MineSphere;
	OnBuildFactory(Volume);
}

void AFactoryBuilding::ActivateFactory()
{
	bIsFactoryActivated = true;
}

void AFactoryBuilding::DeactivateFactory()
{
	bIsFactoryActivated = false;
}

void AFactoryBuilding::OnBuildFactory(int Volume)
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

void AFactoryBuilding::OnTickFactory(float Deltatime)
{
	if (!bIsFactoryActivated) return;
	CurrentInterval +=  Deltatime;
	if (CurrentInterval >= FactoryInfo.MiningCD)
	{
		StartOneMining();
		CurrentInterval = 0;
	}
}

void AFactoryBuilding::OnDestroyFactory()
{
	bIsFactoryActivated = false;
}

bool AFactoryBuilding::StartOneMining()
{
	if (MineSphere->GetCanMine())
	{
		int NewPlayerMine = PlayerData->GetPlayerMineValue() + MineSphere->TryStartOneMine(FactoryEfficiency);
		PlayerData->ChangePlayerMineValue(NewPlayerMine);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("FactoryBuilding: MineSphere is run out of Mine"));
	}
	return false;
}

