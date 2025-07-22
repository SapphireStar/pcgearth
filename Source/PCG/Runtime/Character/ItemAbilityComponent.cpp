
#include "ItemAbilityComponent.h"
#include "Engine/World.h"

UItemAbilityComponent::UItemAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bIsActivated = false;
	bIsInitialized = false;
}

void UItemAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
	OnInitializeAbility();
}

void UItemAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bIsActivated)
	{
		OnTickAbility();
	}
}

void UItemAbilityComponent::OnInitializeAbility()
{
	if (!bIsInitialized)
	{
		bIsInitialized = true;
		
		ReceiveOnInitializeAbility();
	}
}

void UItemAbilityComponent::OnActivateAbility()
{
	if (!bIsActivated)
	{
		bIsActivated = true;
		
		PrimaryComponentTick.bCanEverTick = true;
		
		
		OnAbilityActivated.Broadcast(this);
		
		ReceiveOnActivateAbility();
	}
}

void UItemAbilityComponent::OnTickAbility()
{
	ReceiveOnTickAbility();
}

void UItemAbilityComponent::OnDeactivateAbility()
{
	if (bIsActivated)
	{
		bIsActivated = false;
		
		PrimaryComponentTick.bCanEverTick = false;
		
		
		OnAbilityDeactivated.Broadcast(this);
		
		ReceiveOnDeactivateAbility();
	}
}

void UItemAbilityComponent::OnStartUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
}

void UItemAbilityComponent::OnKeepUsingAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
}

void UItemAbilityComponent::OnCompleteUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
}
