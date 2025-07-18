// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeometryActors/GeneratedDynamicMeshActor.h"
#include "GeometryPlanet.generated.h"

UCLASS()
class PCG_API AGeometryPlanet : public AGeneratedDynamicMeshActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGeometryPlanet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	
	virtual void RebuildGeneratedMesh(UDynamicMesh* TargetMesh) override;
	UFUNCTION(BlueprintCallable)
	void MarkPlanetRefresh(bool bImmediate = false, bool bImmediateEventFrozen = false);
	
	UFUNCTION(BlueprintImplementableEvent)
	void GeneratePlanet(UDynamicMesh* TargetMesh);

	UFUNCTION(BlueprintCallable)
	void GenerateMineAreas();
	void InitializeTexture16Bytes();
	void UpdateTexture16Bytes(bool bFreeData = false);
	UFUNCTION(BlueprintCallable)
	void GenerateMineMaterialTexture();

	void SetPixelValue(int32 Offset, float X, float	Y, float Z, float A);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "PlanetRadius(km)")
	float PlanetRadius = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* PlanetSphereStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Texture)
	UMaterial* Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Texture)
	TArray<FVector> MinePositions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Texture)
	TArray<float> MineRadius;


public:    
	UPROPERTY(EditDefaultsOnly)
	int32 TextureWidth = 4;

	UPROPERTY(EditDefaultsOnly)
	int32 TextureHeight = 1;
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
};
