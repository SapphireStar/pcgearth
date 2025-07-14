// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WFCTileData.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct PCG_API FWFCTile
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString TileName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterial> Material;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FString> Sockets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bShouldCreateRotationVariant;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RotationZ;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.1, ClampMax = 2))
	float Weight;

	UPROPERTY(BlueprintReadOnly)
	bool bIsBaseTile = true;

	void InitializeTileVariant(const FWFCTile& other, TArray<FString> Sockets, float RotationZ);
	
	FString GetSocketForDirection(int dir) const
	{
		return (dir >= 0 && dir < Sockets.Num()) ? Sockets[dir] : TEXT("Empty");
	}
};


UCLASS()
class PCG_API UWFCTileData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FWFCTile> Tiles;
};