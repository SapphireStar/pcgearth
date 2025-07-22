// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Data/PlayerDataComponent.h"
#include "GameFramework/GameModeBase.h"
#include "PCGGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PCG_API APCGGameMode : public AGameModeBase
{
	GENERATED_BODY()

	APCGGameMode();

public:
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UPlayerDataComponent> PlayerData;
};
