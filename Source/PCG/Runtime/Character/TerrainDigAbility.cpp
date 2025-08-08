
#include "TerrainDigAbility.h"
#include "Camera/CameraComponent.h"
#include "GeometryScript/GeometryScriptSelectionTypes.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PCG/Runtime/NewPlanet/GeometryPlanetActor.h"

UTerrainDigAbility::UTerrainDigAbility()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTerrainDigAbility::BeginPlay()
{
	Super::BeginPlay();
}

void UTerrainDigAbility::OnInitializeAbility()
{
	Super::OnInitializeAbility();
}

void UTerrainDigAbility::OnActivateAbility()
{
	Super::OnActivateAbility();
}

void UTerrainDigAbility::OnTickAbility()
{
	Super::OnTickAbility();

}

void UTerrainDigAbility::OnDeactivateAbility()
{
	Super::OnDeactivateAbility();
}

void UTerrainDigAbility::OnStartUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
	Super::OnStartUseAbility(TraceStartComp, Camera);
}

void UTerrainDigAbility::OnKeepUsingAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
	Super::OnKeepUsingAbility(TraceStartComp, Camera);
}

void UTerrainDigAbility::OnCompleteUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
	Super::OnCompleteUseAbility(TraceStartComp, Camera);
	if (!TraceStartComp || !Camera)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid components for digging"));
		return;
	}

	FVector End = TraceStartComp->GetComponentLocation() + Camera->GetForwardVector() * SelectRange;
	TArray<AActor*> ActorsToIgnore;
	FHitResult HitResult;
	
	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TraceStartComp->GetComponentLocation(),
		End,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		5.f);

	if (HitResult.bBlockingHit)
	{
		if (AGeometryPlanetActor* planet = Cast<AGeometryPlanetActor>(HitResult.GetActor()))
		{
			ProcessTerrainDig(planet, HitResult);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit non-planet object, cannot dig here"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid dig target found"));
	}
}

void UTerrainDigAbility::ProcessTerrainDig(AGeometryPlanetActor* Planet, const FHitResult& HitResult)
{
	if (!Planet)
	{
		return;
	}

	FVector ImpactRelativePoint = HitResult.ImpactPoint - Planet->GetActorLocation();
	FGeometryScriptMeshSelection selection;
	
	UGeometryScriptLibrary_MeshSelectionFunctions::SelectMeshElementsInSphere(
		Planet->GetDynamicMeshComponent()->GetDynamicMesh(),
		selection,
		ImpactRelativePoint,
		VertexSelectionTolerance,
		EGeometryScriptMeshSelectionType::Vertices,
		false,
		1
	);
	
	TArray<int32> indicesout;
	selection.ConvertToMeshIndexArray(
		Planet->GetDynamicMeshComponent()->GetDynamicMesh()->GetMeshRef(),
		indicesout);

	if (indicesout.Num() > 0)
	{
		DigTerrain(Planet, indicesout, false);
		UE_LOG(LogTemp, Log, TEXT("Terrain dug successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No suitable terrain found for digging"));
	}
}

void UTerrainDigAbility::DigTerrain(AGeometryPlanetActor* Planet, const TArray<int32>& VertexIndices, bool bForceAdaptive)
{
	if (!Planet || VertexIndices.Num() == 0)
	{
		return;
	}

	auto Mesh = Planet->GetDynamicMeshComponent();
    
	FTerrainAnalysis Analysis = AnalyzeTerrainVariation(Mesh, VertexIndices);
    
	bool bIsValid;
	FVector LowestPos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
		Mesh->GetDynamicMesh(), Analysis.LowestVertexID, bIsValid);
	float LowestLength = LowestPos.Length();
    
	bool bShouldFlatten = bForceAdaptive ? false : Analysis.bShouldFlatten;
	float DigDepthToUse = bShouldFlatten ? 0.0f : CalculateAdaptiveDigDepth(Analysis);
    
	for (int32 VertexID : VertexIndices)
	{
		FVector CurrentPos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
			Mesh->GetDynamicMesh(), VertexID, bIsValid);
        
		if (!bIsValid) continue;
        
		FVector Normal = CurrentPos.GetSafeNormal();
		float NewLength;
        
		if (bShouldFlatten)
		{
			NewLength = LowestLength;
		}
		else
		{
			float CurrentLength = CurrentPos.Length();
			NewLength = FMath::Max(CurrentLength - DigDepthToUse, LowestLength * 0.1f);
		}
        
		UGeometryScriptLibrary_MeshBasicEditFunctions::SetVertexPosition(
			Mesh->GetDynamicMesh(), VertexID, Normal * NewLength, bIsValid);
	}

	Planet->GetDynamicMeshComponent()->UpdateCollision();
}

int UTerrainDigAbility::FindLowestVertex(UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID)
{
	auto DynamicMesh = DynamicMeshComp->GetDynamicMesh();
	bool bHasIdGroup;
	FVector planetPos = DynamicMeshComp->GetOwner()->GetActorLocation();

	float min = FLT_MAX;
	int minID = 0;
	for (auto id : VertexID)
	{
		FVector pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(DynamicMesh, id, bHasIdGroup);
		float dist = FVector::Dist(pos, planetPos);
		if (dist < min)
		{
			min = dist;
			minID = id;
		}
	}
	return minID;
}

FTerrainAnalysis UTerrainDigAbility::AnalyzeTerrainVariation(UDynamicMeshComponent* Mesh, const TArray<int32>& VertexIndices)
{
	FTerrainAnalysis Analysis;
    
	Analysis.LowestVertexID = FindLowestVertex(Mesh, VertexIndices);
	bool bIsValid;
	FVector LowestPos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
		Mesh->GetDynamicMesh(), Analysis.LowestVertexID, bIsValid);
	float LowestDistance = LowestPos.Length();
    
	float TotalDifference = 0.0f;
	float MaxDiff = 0.0f;
    
	for (int32 VertexID : VertexIndices)
	{
		if (VertexID != Analysis.LowestVertexID)
		{
			FVector CurrentPos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
				Mesh->GetDynamicMesh(), VertexID, bIsValid);
			float CurrentDistance = CurrentPos.Length();
			float HeightDiff = CurrentDistance - LowestDistance;
            
			TotalDifference += HeightDiff;
			MaxDiff = FMath::Max(MaxDiff, HeightDiff);
		}
	}
    
	Analysis.AccumulatedDifference = TotalDifference;
	Analysis.MaxHeightDiff = MaxDiff;
	Analysis.AverageHeight = TotalDifference / FMath::Max(1, VertexIndices.Num() - 1);
	Analysis.bShouldFlatten = TotalDifference > HeightVariationThreshold;
    
	return Analysis;
}

float UTerrainDigAbility::CalculateAdaptiveDigDepth(const FTerrainAnalysis& Analysis)
{
	if (Analysis.bShouldFlatten)
	{
		return 0.0f;
	}
    
	float NormalizedVariation = FMath::Clamp(
		Analysis.AccumulatedDifference / HeightVariationThreshold, 
		0.0f, 1.0f
	);
    
	float AdaptiveDepth = FMath::Lerp(MaxDigDepth, MinDigDepth, NormalizedVariation);
    
	return AdaptiveDepth;
}
