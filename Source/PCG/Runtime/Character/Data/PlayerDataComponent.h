// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataTypes.h"
#include "PlayerDataComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerStatusChanged, FPlayerStatusNew, OldValue, const FPlayerStatusNew&, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWoodChanged, int, OldValue, int, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMineChanged, int, OldValue, int, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDataComponentInitialized, UClass*, DataClassType);
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PCG_API UPlayerDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerDataComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void InitializePlayerData(FPlayerStatusNew InitialPlayerStatus);

	UFUNCTION(BlueprintCallable)
	void ChangePlayerWoodValue(int NewValue);

	UFUNCTION(BlueprintPure)
	int GetPlayerWoodValue() const {return PlayerStatus.Wood.Value;}

	UFUNCTION(BlueprintCallable)
	void ChangePlayerMineValue(int NewValue);

	UFUNCTION(BlueprintPure)
	int GetPlayerMineValue() const{return PlayerStatus.Mine.Value;}

public:
	UPROPERTY(BlueprintAssignable)
	FOnPlayerStatusChanged OnPlayerStatusChanged;

	UPROPERTY(BlueprintAssignable)
	FOnWoodChanged OnWoodChanged;

	UPROPERTY(BlueprintAssignable)
	FOnMineChanged OnMineChanged;

	UPROPERTY(BlueprintAssignable)
	FOnDataComponentInitialized OnInitialized;

private:
	FPlayerStatusNew PlayerStatus;
};
