// Fill out your copyright notice in the Description page of Project Settings.


#include "FactoryManager.h"

#include "FactoryBuilding.h"
#include "FactoryCrafter.h"


// Sets default values
AFactoryManager::AFactoryManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AFactoryManager::BeginPlay()
{
	Super::BeginPlay();
	PlayerData = Cast<APCGGameMode>(GetWorld()->GetAuthGameMode())->PlayerData;
	PlayerData->OnTimeZeroGameOver.AddDynamic(this, &AFactoryManager::OnTimeZeroGameover);
}

// Called every frame
void AFactoryManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFactoryManager::OnTimeZeroGameover(UClass* DataClassType, EGameOverType eType)
{
	for (auto Building : SpawnedBuildings)
	{
		Building->Destroy();
	}
	SpawnedBuildings.Empty();
}

void AFactoryManager::BuildMiningFactoryAt(FVector Position, int Volume, AMineSphere* MineSphere, FFactoryInfo Info)
{
	AMiningBuilding* Factory = GetWorld()->SpawnActor<AMiningBuilding>();
	Factory->BuildFactoryAt(Position, Volume,PlayerData->GetPlayerData().MiningFactoryInfo);
	Factory->SetMineSphere(MineSphere);
	Factory->ActivateFactory();
	SpawnedBuildings.Add(Factory);
}

void AFactoryManager::BuildCraftFactoryAt(FVector Position, int Volume, FFactoryInfo Info,
	FFactoryRecipeInfo RecipeInfo)
{
	ACraftingBuilding* Factory = GetWorld()->SpawnActor<ACraftingBuilding>();
	Factory->BuildFactoryAt(Position, Volume, PlayerData->GetPlayerData().CraftingFactoryInfo);
	Factory->SetRecipeInfo(RecipeInfo);
	Factory->ActivateFactory();
	SpawnedBuildings.Add(Factory);
}

