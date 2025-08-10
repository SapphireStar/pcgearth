// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MineSphere.h"
#include "TerrainDataTypes.h"
#include "Components/DynamicMeshComponent.h"
#include "GameFramework/Actor.h"
#include "PCG/Runtime/Planet/ShapeGenerator.h"
#include "GeometryPlanetActor.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnISMInstanceHit, UInstancedStaticMeshComponent*, ISMComponent, int32, Item, float, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlanetInitialized);

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
	UFUNCTION(BlueprintCallable)
	void InitializePlanet(FGeometryPlanetData PlanetData);

	UFUNCTION(BlueprintCallable)
	int GetNextRandomAvaiableVertexID();

	UFUNCTION()
	void ShuffleVertexID();

	UFUNCTION(blueprintCallable)
	bool AddPlanetVertexType(EVertexType eType, int VertexID);

	UFUNCTION(BlueprintCallable)
	bool CheckPlanetVertexType(EVertexType eType, int VertexID);

#pragma region  Terrain
	UFUNCTION(BlueprintCallable)
	void MarkPlanetRefresh(bool bImmediate = false, bool bImmediateEventFrozen = false);
	
	UFUNCTION(BlueprintImplementableEvent)
	void GeneratePlanet(UDynamicMesh* TargetMesh);

	UFUNCTION(BlueprintCallable)
	UDynamicMesh* ApplyNoiseToPlanet();

	UFUNCTION(BlueprintCallable)
	void SpawnCraters();
	
	UFUNCTION(BlueprintCallable)
	void ApplyCraterToPlanet();

#pragma endregion

	UFUNCTION(BlueprintCallable)
	void SpawnStoneMineSpheres();

	UFUNCTION(BlueprintCallable)
	void SpawnOreMineSpheres();

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
	FIntVector PlanetResolution = FIntVector(100, 100, 100);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* PlanetSphereStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterial* PlanetMaterial;
		
	UPROPERTY(BlueprintReadWrite)
	FRandomStream RandomStream;

#pragma region Mineral
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMineSphereSpawnConfiguration MineConfiguration;
	
	UPROPERTY()
	TArray<FVector> MinePositions;
	
	UPROPERTY()
	TArray<float> MineRadius;
	
	UPROPERTY()
	TArray<AMineSphere*> MineSpheres;
	
private:
	float UpdateTextureCD = 1;
	float UpdateTextureCounter = 0;
	bool bIsTextureInitialized = false;
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterialInstance;
	int32 TextureWidth = 4;
	int32 TextureHeight = 1;
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

	FCraterSpawnConfiguration CraterSpawnConfiguration;

	UPROPERTY(Blueprintreadwrite, EditAnywhere)
	TArray<FCraterData> CratersData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	FShapeSettings NoiseShapeSettings;

	UPROPERTY()
	TObjectPtr<UShapeGenerator> NoiseShapeGenerator;
#pragma	endregion

#pragma region Foliage
public:
	UFUNCTION(BlueprintCallable)
	void InitializeISMFoliage(UInstancedStaticMeshComponent* ISMComponent);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnGetHitByLaser(UInstancedStaticMeshComponent* ISMComponent, int32 ItemIndex, float Damage);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInstancedStaticMeshComponent> ISMFoliage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> ISMFoliageItemsHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FoliageAmount = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShouldSpawnFoliage = true;
#pragma  endregion

#pragma region VertexData
private:
	TArray<int> VertexTypeData;

	TArray<int> AvailableRandomVertex;

	int CurTopRandomVertexIndex;

#pragma endregion

#pragma  region Delegates
public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnISMInstanceHit OnISMInstanceHit;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnPlanetInitialized OnPlanetInitialized;

#pragma endregion
};
