// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainFace.h"

#include "GeometryScript/ListUtilityFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshBooleanFunctions.h"
#include "GeometryScript/MeshNormalsFunctions.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "Tasks/Task.h"

// Sets default values
ATerrainFace::ATerrainFace()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ATerrainFace::InitializeTerrain(TObjectPtr<UShapeGenerator>& ShapeGenerator, int Resolution, FVector Localup)
{
	this->ShapeGenerator = ShapeGenerator;
	this->Resolution = Resolution;
	this->Localup = Localup;
}

void ATerrainFace::ConstructMesh()
{
	AxisA = FVector(Localup.Y, Localup.Z, Localup.X);
	AxisB = FVector::CrossProduct( AxisA,Localup);
	DynamicMeshComponent->GetDynamicMesh()->Reset();
	TArray<FVector> Vertices;
	Vertices.SetNum(Resolution * Resolution);
	TArray<FIntVector> Triangles;
	Triangles.Init(FIntVector::ZeroValue, (Resolution - 1) * (Resolution - 1) * 6);
	int triIndex = 0;

	for (int y = 0; y < Resolution; y++)
	{
		for (int x = 0; x < Resolution; x++)
		{
			int i = y * Resolution + x;
			FVector2D percent = FVector2D(x, y) / (Resolution - 1);
			FVector pointOnUnitCube = (Localup + (percent.X - 0.5f) * 2 * AxisA + (percent.Y - 0.5f) * 2 * AxisB);
			FVector pointOnUnitSphere = pointOnUnitCube.GetSafeNormal();
			pointOnUnitSphere = ShapeGenerator->CalculatePointOnPlanet(pointOnUnitSphere);
			Vertices[i] = pointOnUnitSphere;
			int NewIndex;

			
		}
	}

	FGeometryScriptVectorList VertexList;
	FGeometryScriptIndexList VertexIndexList;
	UGeometryScriptLibrary_ListUtilityFunctions::ConvertArrayToVectorList(Vertices, VertexList);
	UGeometryScriptLibrary_MeshBasicEditFunctions::AddVerticesToMesh(
	DynamicMeshComponent->GetDynamicMesh(),
	VertexList,
	VertexIndexList
);

	for (int y = 0; y < Resolution; y++)
	{
		for (int x = 0; x < Resolution; x++)
		{
			int i = y * Resolution + x;

			//生成每一个正方形tile的三角形index
			if (x != Resolution - 1 && y != Resolution - 1)
			{
				Triangles[triIndex] = FIntVector(i, i + Resolution + 1, i + Resolution);

				Triangles[triIndex + 1] = FIntVector(i, i + 1, i + Resolution + 1);
				
				int newIndex;
				UGeometryScriptLibrary_MeshBasicEditFunctions::AddTriangleToMesh(
					DynamicMeshComponent->GetDynamicMesh(),
					Triangles[triIndex],
					newIndex
				);
				UGeometryScriptLibrary_MeshBasicEditFunctions::AddTriangleToMesh(
					DynamicMeshComponent->GetDynamicMesh(),
					Triangles[triIndex + 1],
					newIndex
				);
				
				triIndex += 2;

			}
		}
	}


	FGeometryScriptIndexList AllVertices;
	bool hasgaps;
	UGeometryScriptLibrary_MeshQueryFunctions::GetAllVertexIDs(
		DynamicMeshComponent->GetDynamicMesh(),
		AllVertices,
		hasgaps
	);

	UGeometryScriptLibrary_MeshNormalsFunctions::ComputeSplitNormals(
		DynamicMeshComponent->GetDynamicMesh(),
		FGeometryScriptSplitNormalsOptions(),
		FGeometryScriptCalculateNormalsOptions()
	);

	DynamicMeshComponent->NotifyMeshModified();
	DynamicMeshComponent->NotifyMeshUpdated();
	
	/*DynamicMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics); // 只需要查询，不需要物理
	DynamicMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	DynamicMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	// 启用复杂碰撞作为简单碰撞（重要！）
	DynamicMeshComponent->EnableComplexAsSimpleCollision();

	// 更新碰撞
	DynamicMeshComponent->UpdateCollision();*/
}

// Called when the game starts or when spawned
void ATerrainFace::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ATerrainFace::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATerrainFace::RebuildGeneratedMesh(UDynamicMesh* TargetMesh)
{
	Super::RebuildGeneratedMesh(TargetMesh);
}
