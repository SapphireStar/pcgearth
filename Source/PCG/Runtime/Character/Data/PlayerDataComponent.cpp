// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerDataComponent.h"


// Sets default values for this component's properties
UPlayerDataComponent::UPlayerDataComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayerDataComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPlayerDataComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPlayerDataComponent::InitializePlayerData(FPlayerDataContainer InitialPlayerData)
{
	PlayerStatus = InitialPlayerData.PlayerStatus;
	SystemStatus = InitialPlayerData.SystemStatus;
	OnInitialized.Broadcast(UPlayerDataComponent::StaticClass());
	OnPlayerStatusChanged.Broadcast(PlayerStatus, PlayerStatus);
	PlayerDataContainer = InitialPlayerData;
}

void UPlayerDataComponent::ChangePlayerWoodValue(int NewValue)
{
	int WoodOld = PlayerStatus.Wood.Value;
	PlayerStatus.Wood.Value = NewValue;
	OnWoodChanged.Broadcast(WoodOld, NewValue);
}

void UPlayerDataComponent::ChangePlayerStoneValue(int NewValue)
{
	int MineOld = PlayerStatus.Stone.Value;
	PlayerStatus.Stone.Value = NewValue;
	OnStoneChanged.Broadcast(MineOld, NewValue);
}

void UPlayerDataComponent::ChangePlayerOreValue(int NewValue)
{
	int OreOld = PlayerStatus.Ore.Value;
	PlayerStatus.Ore.Value = NewValue;
	OnOreChanged.Broadcast(OreOld, NewValue);
}

void UPlayerDataComponent::ChangePlayerMetalValue(int NewValue)
{
	int MetalOld = PlayerStatus.Metal.Value;
	PlayerStatus.Metal.Value = NewValue;
	OnMetalChanged.Broadcast(MetalOld, NewValue);
}

void UPlayerDataComponent::ChangePlayerRemainDaysValue(int NewValue)
{
	int RemainDaysOld = SystemStatus.RemainDays;
	SystemStatus.RemainDays = NewValue;
	OnRemainDaysChanged.Broadcast(RemainDaysOld, NewValue);
}

void UPlayerDataComponent::ChangePlayerCurrentTimeValue(int NewValue)
{
	int CurrentTimeOld = SystemStatus.CurrentTime;
	SystemStatus.CurrentTime = NewValue;
	OnCurrentTimeChanged.Broadcast(CurrentTimeOld, NewValue);
}

void UPlayerDataComponent::BroadcastTimeZeroGameover()
{
	OnTimeZeroGameOver.Broadcast(UPlayerDataComponent::StaticClass());
}

