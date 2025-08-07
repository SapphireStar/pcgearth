// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FTUEData.generated.h"

UENUM(BlueprintType)
enum class ETutorialType: uint8
{
	ETT_InitialTutorial		UMETA(DisplayName = "Initial Tutorial"),
	ETT_CraftTutorial		UMETA(DisplayName = "Craft Tutorial"),
};

USTRUCT(BlueprintType, Blueprintable)
struct PCG_API FTutorialChainData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETutorialType TutorialType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<AActor>> TutorialActorClass;
};

UCLASS(BlueprintType, Blueprintable)
class PCG_API UFTUEData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTutorialChainData> TutorialChains;

	UFUNCTION(BlueprintCallable)
	FTutorialChainData GetTutorialChain(ETutorialType eType);
};
