// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShapeGenerator.h"
#include "GeometryActors/GeneratedDynamicMeshActor.h"
#include "GeometryScript/MeshNormalsFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "ProceduralMeshComponent/Public/ProceduralMeshComponent.h"
#include "TerrainFace.generated.h"

UCLASS()
class PCG_API ATerrainFace : public AGeneratedDynamicMeshActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATerrainFace();
	void InitializeTerrain(TObjectPtr<UShapeGenerator>& ShapeGenerator, int Resolution, FVector Localup);
	void ConstructMesh();

public:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	int Resolution = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	FVector Localup = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	FVector AxisA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	FVector AxisB;

	UPROPERTY()
	TObjectPtr<UShapeGenerator> ShapeGenerator;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void DebugDynamicMesh(UDynamicMesh* Mesh, UDynamicMeshComponent* MeshComp)
	{
		// 1. 检查网格统计
		int32 VertexCount = UGeometryScriptLibrary_MeshQueryFunctions::GetNumVertexIDs(Mesh);
		int32 TriangleCount = UGeometryScriptLibrary_MeshQueryFunctions::GetNumTriangleIDs(Mesh);
    
		UE_LOG(LogTemp, Log, TEXT("Vertices: %d, Triangles: %d"), VertexCount, TriangleCount);
    
		if (VertexCount == 0 || TriangleCount == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Mesh is empty!"));
			return;
		}
		
    
		// 3. 更新渲染组件
		if (MeshComp)
		{
			MeshComp->SetDynamicMesh(Mesh);
			MeshComp->NotifyMeshUpdated();
        
			// 4. 强制更新边界框
			MeshComp->UpdateBounds();
        
			// 5. 标记为需要重新渲染
			MeshComp->MarkRenderStateDirty();
		}
    
		// 6. 检查边界框
		FBox BoundingBox = UGeometryScriptLibrary_MeshQueryFunctions::GetMeshBoundingBox(Mesh);
		UE_LOG(LogTemp, Log, TEXT("Bounding Box: Min=%s, Max=%s"), 
			   *BoundingBox.Min.ToString(), 
			   *BoundingBox.Max.ToString());
	}

protected:
	virtual void RebuildGeneratedMesh(UDynamicMesh* TargetMesh) override;
};
