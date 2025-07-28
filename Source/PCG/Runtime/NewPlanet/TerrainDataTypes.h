#pragma once

#include "CoreMinimal.h"
#include "TerrainDataTypes.generated.h"

UENUM(BlueprintType)
enum class EVertexType : uint8
{
	EVT_None = 0,
	EVT_MineSphere = 1,
	EVT_Factory = 2,
	EVT_Foliage = 4,
	EVT_Crater = 8,
};

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

USTRUCT(BlueprintType)
struct PCG_API FMineSphereSpawnConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = 500, ClampMax = 3000))
	float RadiusMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = 1000, ClampMax =3000))
	float RadiusMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxMineSphereAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Seed;
};

USTRUCT(BlueprintType)
struct PCG_API FCraterSpawnConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CraterMinRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CraterMaxRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CraterMinDepth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CraterMaxDepth;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CraterMinRimHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CraterMaxRimHeight;
};

USTRUCT(BlueprintType)
struct PCG_API FGeometryPlanetData : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlanetRadius = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector PlanetResolution = FIntVector(100, 100, 100);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterial> PlanetMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMesh> PlanetSphereStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int RandomSeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMineSphereSpawnConfiguration MineConfiguration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCraterSpawnConfiguration CraterConfiguration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FoliageAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShouldSpawnFoliage = true;
};
