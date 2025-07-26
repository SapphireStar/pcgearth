#pragma once

#include "CoreMinimal.h"
#include "TerrainDataTypes.generated.h"

USTRUCT(BlueprintType)
struct PCG_API FCraterData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CraterCenter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CraterRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CraterDepth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CraterRimHeight;
};
