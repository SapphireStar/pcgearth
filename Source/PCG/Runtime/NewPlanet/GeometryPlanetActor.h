// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MineSphere.h"
#include "TerrainDataTypes.h"
#include "Components/DynamicMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GeometryPlanetActor.generated.h"

USTRUCT(BlueprintType)
struct PCG_API FMineSphereSpawnConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = 500, ClampMax = 1000))
	float RadiusMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = 1000, ClampMax =3000))
	float RadiusMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Seed;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnISMInstanceHit, UInstancedStaticMeshComponent*, ISMComponent, int32, Item, float, Damage);

UCLASS()
class PCG_API AGeometryPlanetActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGeometryPlanetActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
#pragma region  Terrain
	virtual void RebuildGeneratedMesh(UDynamicMesh* TargetMesh);
	UFUNCTION(BlueprintCallable)
	void MarkPlanetRefresh(bool bImmediate = false, bool bImmediateEventFrozen = false);
	
	UFUNCTION(BlueprintImplementableEvent)
	void GeneratePlanet(UDynamicMesh* TargetMesh);

	UFUNCTION(BlueprintCallable)
	void ApplyNoiseToPlanet();
	
	UFUNCTION(BlueprintCallable)
	void ApplyCraterToPlanet();

#pragma endregion

	UFUNCTION(BlueprintCallable)
	void SpawnMineSpheres();

	UFUNCTION(BlueprintCallable)
	void GenerateMineAreas();
	
	UFUNCTION(BlueprintCallable)
	void GenerateMineMaterialTexture();
	
	UFUNCTION(BlueprintCallable)
	void UpdateMineAreas();
	
	void InitializeTexture16Bytes();
	
	void UpdateTexture16Bytes(bool bFreeData = false);
	
	UDynamicMeshComponent* GetDynamicMeshComponent();

	void SetPixelValue(int32 Offset, float X, float	Y, float Z, float A);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "PlanetRadius(km)")
	float PlanetRadius = 10.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* PlanetSphereStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterial* Material;

#pragma region Mineral
public:    
	UPROPERTY(EditDefaultsOnly)
	int32 TextureWidth = 4;

	UPROPERTY(EditDefaultsOnly)
	int32 TextureHeight = 1;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMineSphereSpawnConfiguration MineConfiguration;
	
	UPROPERTY()
	TArray<FVector> MinePositions;
	
	UPROPERTY()
	TArray<float> MineRadius;
	
	UPROPERTY()
	TArray<AMineSphere*> MineSpheres;
	
	FRandomStream RandomStream;

	
private:
	bool bIsTextureInitialized = false;
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterialInstance;
	uint8* TextureData;
	float* TextureDataFloat;
	uint32 TextureDataSize = 4;
	uint32 TextureDataSqrtSize;
	uint32 TextureTotalPixels;
	UPROPERTY()
	UTexture2D* DynamicTexture;
	FUpdateTextureRegion2D* TextureRegion;
#pragma endregion

#pragma  region Terrain
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UDynamicMeshComponent> DynamicMeshComponent;

	UPROPERTY(Blueprintreadwrite, EditAnywhere)
	TArray<FCraterData> CratersData;
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	FShapeSettings NoiseShapeSettings;

	UPROPERTY()
	TObjectPtr<UShapeGenerator> NoiseShapeGenerator;*/
#pragma	endregion



#pragma region Foliage
public:
	UFUNCTION(BlueprintCallable)
	void InitializeISMFoliage(UInstancedStaticMeshComponent* ISMComponent);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnGetHitByLaser(UInstancedStaticMeshComponent* ISMComponent, int32 ItemIndex, float Damage);
public:
	UPROPERTY(BlueprintAssignable)
	FOnISMInstanceHit OnISMInstanceHit;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInstancedStaticMeshComponent> ISMFoliage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> ISMFoliageItemsHealth;
#pragma  endregion 
};
