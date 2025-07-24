// Fill out your copyright notice in the Description page of Project Settings.


#include "PCGGameMode.h"

APCGGameMode::APCGGameMode()
{
	PlayerData = CreateDefaultSubobject<UPlayerDataComponent>(TEXT("PlayerData"));

}

void APCGGameMode::BeginPlay()
{
	/*PlayerData = NewObject<UPlayerDataComponent>();
	AddOwnedComponent(PlayerData);
	PlayerData->RegisterComponent();*/
	Super::BeginPlay();
}
