// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Data/PlayerDataComponent.h"
#include "GameFramework/GameModeBase.h"
#include "PCGGameMode.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShowPopup, EPopupType, PopupType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHidePopup);
UCLASS()
class PCG_API APCGGameMode : public AGameModeBase
{
	GENERATED_BODY()

	APCGGameMode();

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void ShowPopup(EPopupType PopupType);

	UFUNCTION(BlueprintCallable)
	void HidePopup();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UPlayerDataComponent> PlayerData;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnShowPopup OnShowPopup;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnHidePopup OnHidePopup;

	bool bIsShowingPopup;
};
