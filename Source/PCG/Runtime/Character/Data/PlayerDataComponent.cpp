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
}

void UPlayerDataComponent::ChangePlayerWoodValue(int NewValue)
{
	int WoodOld = PlayerStatus.Wood.Value;
	PlayerStatus.Wood.Value = NewValue;
	OnWoodChanged.Broadcast(WoodOld, NewValue);
}

void UPlayerDataComponent::ChangePlayerMineValue(int NewValue)
{
	int MineOld = PlayerStatus.Mine.Value;
	PlayerStatus.Mine.Value = NewValue;
	OnMineChanged.Broadcast(MineOld, NewValue);
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

