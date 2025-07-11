// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DynamicMeshActor.h"
#include "GameFramework/Actor.h"
#include "GeometryScriptingEditor/Public/GeometryActors/GeneratedDynamicMeshActor.h"
#include "PlaneEditor.generated.h"

UCLASS()
class PCG_API APlaneEditor : public AGeneratedDynamicMeshActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlaneEditor();
protected:
	// UPROPERTY()
	// TObjectPtr<UDynamicMesh> DynamicMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Configs")
	class UStaticMesh* DefaultStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Configs")
	TObjectPtr<UTexture2D>  HeightMapTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Configs")
	TObjectPtr<UMaterial> DefaultMaterial;

	UPROPERTY()
	TArray<FLinearColor> HeightMapPixelColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Configs")
	float Bump = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Configs")
	float Height = 100.f;

	bool bIsHeightMapLoaded = false;
    	
	virtual void BeginPlay() override;

	virtual void Tick( float DeltaTime ) override;


public:
	UFUNCTION(BlueprintCallable, Category = "Mesh Setup")
	void LoadDefaultMesh();

	UFUNCTION(BlueprintCallable, Category = "Mesh Setup")
	void SetMeshFromAsset(UStaticMesh* MeshAsset);

	UFUNCTION(BlueprintCallable, Category = "Mesh Modify")
	void ModifyVerticesWithBump();

	UFUNCTION(BlueprintCallable, Category = "Mesh Modify")
	void ModifyVerticesWithHeightMap();

protected:
	virtual void RebuildGeneratedMesh(UDynamicMesh* TargetMesh) override;
};
