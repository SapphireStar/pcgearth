// ItemAbilityComponent.cpp - 更新后的实现

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
	
	// 只有在激活状态下才执行Tick逻辑
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
		UE_LOG(LogTemp, Log, TEXT("Ability '%s' initialized"), *AbilityName);
		
		// 调用蓝图事件
		ReceiveOnInitializeAbility();
	}
}

void UItemAbilityComponent::OnActivateAbility()
{
	if (!bIsActivated)
	{
		bIsActivated = true;
		
		// 如果需要Tick更新，启用Tick
		PrimaryComponentTick.bCanEverTick = true;
		
		UE_LOG(LogTemp, Log, TEXT("Ability '%s' activated"), *AbilityName);
		
		// 广播激活事件
		OnAbilityActivated.Broadcast(this);
		
		// 调用蓝图事件
		ReceiveOnActivateAbility();
	}
}

void UItemAbilityComponent::OnTickAbility()
{
	// 子类可以重写此函数实现持续逻辑
	// 调用蓝图事件
	ReceiveOnTickAbility();
}

void UItemAbilityComponent::OnDeactivateAbility()
{
	if (bIsActivated)
	{
		bIsActivated = false;
		
		// 停用Tick以优化性能
		PrimaryComponentTick.bCanEverTick = false;
		
		UE_LOG(LogTemp, Log, TEXT("Ability '%s' deactivated"), *AbilityName);
		
		// 广播停用事件
		OnAbilityDeactivated.Broadcast(this);
		
		// 调用蓝图事件
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
