#include "PlaneEditor.h"
#include "CoreMinimal.h"
#include "DrawDebugHelpers.h"
#include "Helper.h"
#include "StaticMeshResources.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Chaos/PBDJointConstraintData.h"
#include "Components/DynamicMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "Engine/World.h"
#include "GeometryScript/CollisionFunctions.h"
#include "GeometryScript/ListUtilityFunctions.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshBooleanFunctions.h"
#include "GeometryScript/MeshDecompositionFunctions.h"
#include "GeometryScript/MeshDeformFunctions.h"
#include "GeometryScript/MeshMaterialFunctions.h"
#include "GeometryScript/MeshModelingFunctions.h"
#include "GeometryScript/MeshNormalsFunctions.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "GeometryScript/MeshRemeshFunctions.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "GeometryScript/MeshSelectionQueryFunctions.h"
#include "GeometryScript/MeshSimplifyFunctions.h"
#include "GeometryScript/MeshSpatialFunctions.h"
#include "GeometryScript/MeshTransformFunctions.h"
#include "GeometryScript/MeshUVFunctions.h"
#include "GeometryScript/SceneUtilityFunctions.h"
#include "PhysicsEngine/BodySetup.h"
#include "UObject/SavePackage.h"

APlaneEditor::APlaneEditor()
{
	PrimaryActorTick.bCanEverTick = true;
	LoadDefaultMesh();
	
}

void APlaneEditor::BeginPlay()
{
	Super::BeginPlay();
	Helper::CacheTextureSampler(HeightMapTexture, HeightMapPixelColors);
}

void APlaneEditor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ModifyVerticesWithHeightMap();
}

void APlaneEditor::LoadDefaultMesh()
{
	if (DefaultStaticMesh && DynamicMeshComponent)
	{
		EGeometryScriptOutcomePins Outcome;
		UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromStaticMesh(
			DefaultStaticMesh,
			DynamicMeshComponent->GetDynamicMesh(),
			FGeometryScriptCopyMeshFromAssetOptions(),
			FGeometryScriptMeshReadLOD(),
			Outcome);
		
		DynamicMeshComponent->UpdateCollision();
		UE_LOG(LogTemp, Warning, TEXT("Loaded default mesh: %s"), *DefaultStaticMesh->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No default mesh specified or components not initialized"));
	}
}

void APlaneEditor::SetMeshFromAsset(UStaticMesh* MeshAsset)
{
	if (MeshAsset)
	{
		EGeometryScriptOutcomePins Outcome;
		UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromStaticMesh(
			MeshAsset,
			DynamicMeshComponent->GetDynamicMesh(),
			FGeometryScriptCopyMeshFromAssetOptions(),
			FGeometryScriptMeshReadLOD(),
			Outcome);
		if (Outcome == EGeometryScriptOutcomePins::Success)
		{
			UE_LOG(LogTemp, Warning, TEXT("Set mesh from asset: %s"), *MeshAsset->GetName());
		}


	}

	
}

void APlaneEditor::ModifyVerticesWithBump()
{
	FGeometryScriptVectorList VertexPositions;
	bool bHasGapIDs;
	UGeometryScriptLibrary_MeshQueryFunctions::GetAllVertexPositions(DynamicMeshComponent->GetDynamicMesh(), VertexPositions, false, bHasGapIDs);
	TSharedPtr<TArray<FVector>> VertexReader = VertexPositions.List;
	for (int i = 0; i< VertexPositions.List->Num(); ++i)
	{
		FVector& vertex = (*VertexReader)[i];
		float dist =FVector::Dist( FVector(vertex.X, vertex.Y, 0), FVector(DynamicMeshComponent->GetComponentLocation().X,  DynamicMeshComponent->GetComponentLocation().Y, 0) );
		float h = 1.f - FMath::Clamp(dist / Bump, 0.f, 1.f);
		h = h*h*h*(h*(h*6-15)+10);
		vertex.Z = h * 128;
	}

	UGeometryScriptLibrary_MeshBasicEditFunctions::SetAllMeshVertexPositions(DynamicMeshComponent->GetDynamicMesh(), VertexPositions);
	DynamicMeshComponent->NotifyMeshUpdated();
}

void APlaneEditor::ModifyVerticesWithHeightMap()
{
	if (!HeightMapTexture)
	{
		return;
	}
	
	FGeometryScriptVectorList VertexPositions;
	bool bHasGapIDs;
	UGeometryScriptLibrary_MeshQueryFunctions::GetAllVertexPositions(DynamicMeshComponent->GetDynamicMesh(), VertexPositions, false, bHasGapIDs);
	TSharedPtr<TArray<FVector>> VertexReader = VertexPositions.List;
	for (int i = 0; i< VertexPositions.List->Num(); ++i)
	{
		FVector& vertex = (*VertexReader)[i];
		float texX =( vertex.X + DynamicMeshComponent->Bounds.GetBox().GetSize().X / 2) / DynamicMeshComponent->Bounds.GetBox().GetSize().X;
		float texY = ( vertex.Y + DynamicMeshComponent->Bounds.GetBox().GetSize().Y / 2) / DynamicMeshComponent->Bounds.GetBox().GetSize().Y;

		vertex.Z = Helper::BilinearSample(HeightMapTexture, HeightMapPixelColors, texX, texY).R * Height;
	}
	UGeometryScriptLibrary_MeshBasicEditFunctions::SetAllMeshVertexPositions(DynamicMeshComponent->GetDynamicMesh(), VertexPositions);
	DynamicMeshComponent->NotifyMeshUpdated();
}

void APlaneEditor::RebuildGeneratedMesh(UDynamicMesh* TargetMesh)
{
	Super::RebuildGeneratedMesh(TargetMesh);
	LoadDefaultMesh();
	ModifyVerticesWithHeightMap();
}
