// Fill out your copyright notice in the Description page of Project Settings.


#include "PCGGameMode.h"

APCGGameMode::APCGGameMode()
{
	

}

void APCGGameMode::BeginPlay()
{
	PlayerData = NewObject<UPlayerDataComponent>();
	PlayerData->RegisterComponent();
	Super::BeginPlay();
}
