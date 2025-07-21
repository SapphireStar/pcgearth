// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemAbilityComponent.h"


// Sets default values for this component's properties
UItemAbilityComponent::UItemAbilityComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UItemAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	OnInitializeAbility();
}


// Called every frame
void UItemAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UItemAbilityComponent::OnInitializeAbility()
{
}

void UItemAbilityComponent::OnActivateAbility()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UItemAbilityComponent::OnTickAbility()
{
	
}

void UItemAbilityComponent::OnDeactivateAbility()
{
	PrimaryComponentTick.bCanEverTick = false;
}

