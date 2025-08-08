
#include "ItemAbilityComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PCG/Runtime/PCGGameMode.h"

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
	FName Abilityname = UEnum::GetValueAsName(AbilityType);
	
	if (GetWorld())
	{
		UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("%s Initialized"), *Abilityname.ToString()), true);
	}
	PlayerData = Cast<APCGGameMode>(GetWorld()->GetAuthGameMode())->PlayerData;
}

void UItemAbilityComponent::OnActivateAbility()
{
	if (!bIsActivated)
	{
		bIsActivated = true;
		
		PrimaryComponentTick.bCanEverTick = true;
		
		ReceiveOnActivateAbility();
	}
	FName Abilityname = UEnum::GetValueAsName(AbilityType);
	if (GetWorld())
			UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("%s Activated"), *Abilityname.ToString()), true);
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

		ReceiveOnDeactivateAbility();
	}
	FName Abilityname = UEnum::GetValueAsName(AbilityType);
	if (GetWorld())
		UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("%s Deactivated"), *Abilityname.ToString()), true);
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

void UItemAbilityComponent::OnCancelUseAbility()
{
}
