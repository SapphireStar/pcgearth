// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFCSolver.h"
#include "Engine/DataAsset.h"
#include "WFCData.generated.h"

/**
 * 
 */
USTRUCT(Blueprintable)
struct FBlockSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterial> Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSocketInfo SocketInfo;
};
UCLASS(Blueprintable, BlueprintType)
class PCG_API UWFCData : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FBlockSettings> BlockSettings;
};
