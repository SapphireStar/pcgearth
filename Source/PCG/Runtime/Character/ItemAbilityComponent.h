#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/DataTypes.h"
#include "ItemAbilityComponent.generated.h"

UCLASS(Blueprintable)
class PCG_API UItemAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UItemAbilityComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", meta = (CallInEditor = "true"))
	void ReceiveOnInitializeAbility();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", meta = (CallInEditor = "true"))
	void ReceiveOnActivateAbility();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", meta = (CallInEditor = "true"))
	void ReceiveOnTickAbility();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", meta = (CallInEditor = "true"))
	void ReceiveOnDeactivateAbility();
	
	UFUNCTION(BlueprintCallable)
	virtual void OnInitializeAbility();

	UFUNCTION(BlueprintCallable)
	virtual void OnActivateAbility();

	UFUNCTION(BlueprintCallable)
	virtual void OnTickAbility();

	UFUNCTION(BlueprintCallable)
	virtual void OnDeactivateAbility();

	UFUNCTION(BlueprintPure)
	bool IsActivated() const { return bIsActivated; }

	UFUNCTION(BlueprintPure)
	bool IsInitialized() const { return bIsInitialized; }

	UFUNCTION(BlueprintPure)
	FString GetAbilityName() const { return AbilityName; }

	UFUNCTION(BlueprintPure)
	FString GetAbilityDescription() const { return AbilityDescription; }

	UFUNCTION(BlueprintCallable)
	virtual void OnStartUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera);

	UFUNCTION(BlueprintCallable)
	virtual void OnKeepUsingAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera);

	UFUNCTION(BlueprintCallable)
	virtual void OnCompleteUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera);

public:
	UPROPERTY(BlueprintReadOnly)
	bool bIsActivated = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsInitialized = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EAbilityType AbilityType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Info")
	FString AbilityName = TEXT("Base Ability");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Info", meta = (MultiLine = true))
	FString AbilityDescription = TEXT("Base ability component");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Info")
	TObjectPtr<class UTexture2D> AbilityIcon;
};