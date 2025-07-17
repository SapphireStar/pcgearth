// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestMineMaterialTexture.generated.h"

UCLASS()
class PCG_API ATestMineMaterialTexture : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATestMineMaterialTexture();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable)
	UTexture2D* GenerateMineMaterialTexture(TArray<FVector>& Positions, TArray<float>& Values);
	void GenerateMineMaterialTexture();
	UFUNCTION(BlueprintCallable, Category = "Dynamic Texture")
	void FillTexture(FLinearColor Color);
private:
	void InitializeTexture();
	void InitializeTexture16Bytes();
	//Update Texture Object from Texture Data
	void UpdateTexture(bool bFreeData = false);
	void UpdateTexture16Bytes(bool bFreeData = false);

	void SetPixelValue(int32 X, FColor Color);
	void SetPixelValue(int32 Offset, float X, float	Y, float Z, float A);
	void SetPositionAndRadius(int32 Offset, int32 X, int32 Y, int32 Z, int32 Radius);


protected:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* StaticMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Texture)
	UMaterial* Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Texture)
	TArray<FVector> Positions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Texture)
	TArray<float> Values;

public:    
	UPROPERTY(EditDefaultsOnly)
	int32 TextureWidth = 4;

	UPROPERTY(EditDefaultsOnly)
	int32 TextureHeight = 1;
private:
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterialInstance;
	// Array that contains the Texture Data
	uint8* TextureData;
	float* TextureDataFloat;
	
	// Total Bytes of Texture Data
	uint32 TextureDataSize = 1;

	// Texture Data Sqrt Size
	uint32 TextureDataSqrtSize;

	// Total Count of Pixels in Texture
	uint32 TextureTotalPixels;
	// Texture Object
	UPROPERTY()
	UTexture2D* DynamicTexture;

	// Update Region Struct
	FUpdateTextureRegion2D* TextureRegion;


};
