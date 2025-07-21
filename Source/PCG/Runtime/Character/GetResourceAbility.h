// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemAbilityComponent.h"
#include "GetResourceAbility.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PCG_API UGetResourceAbility : public UItemAbilityComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGetResourceAbility();

protected:
	virtual void BeginPlay() override;

public:
	// 重写基类的生命周期函数
	virtual void OnInitializeAbility() override;
	virtual void OnActivateAbility() override;
	virtual void OnTickAbility() override;
	virtual void OnDeactivateAbility() override;
	
	virtual void OnStartUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera) override;
	virtual void OnKeepUsingAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera) override;
	virtual void OnCompleteUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera) override;

private:
	float Range = 5000.f;
};
