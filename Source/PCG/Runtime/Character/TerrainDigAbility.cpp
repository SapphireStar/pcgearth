
#include "TerrainDigAbility.h"
#include "Camera/CameraComponent.h"
#include "GeometryScript/GeometryScriptSelectionTypes.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PCG/Runtime/NewPlanet/GeometryPlanet.h"

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
	UE_LOG(LogTemp, Log, TEXT("TerrainDigAbility initialized"));
}

void UTerrainDigAbility::OnActivateAbility()
{
	Super::OnActivateAbility();
	UE_LOG(LogTemp, Log, TEXT("TerrainDigAbility activated - Ready to dig!"));
	
	// 激活时可以显示挖掘预览等
}

void UTerrainDigAbility::OnTickAbility()
{
	Super::OnTickAbility();
	// 如果需要持续更新的逻辑，可以在这里实现
	// 例如：显示挖掘预览、更新UI等
}

void UTerrainDigAbility::OnDeactivateAbility()
{
	Super::OnDeactivateAbility();
	UE_LOG(LogTemp, Log, TEXT("TerrainDigAbility deactivated"));
	
	// 停用时清理预览等
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
		DigTerrain(Planet, indicesout);
		UE_LOG(LogTemp, Log, TEXT("Terrain dug successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No suitable terrain found for digging"));
	}
}

void UTerrainDigAbility::DigTerrain(AGeometryPlanetActor* Planet, const TArray<int32>& VertexIndices)
{
	if (!Planet || VertexIndices.Num() == 0)
	{
		return;
	}

	int LowestVertexID = FindLowestVertex(Planet->GetDynamicMeshComponent(), VertexIndices);
	bool bIsValidVertex;
	auto mesh = Planet->GetDynamicMeshComponent()->GetDynamicMesh();

	auto lowestPos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
		mesh, LowestVertexID, bIsValidVertex);
	float lowestLength = lowestPos.Length();

	for (int i : VertexIndices)
	{
		if (i != LowestVertexID)
		{
			auto pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
				mesh, i, bIsValidVertex);
			FVector normal = pos.GetSafeNormal();
			
			
			float newLength = FMath::Max(lowestLength - DigDepth, lowestLength * 0.1f); //防止挖得太深
			UGeometryScriptLibrary_MeshBasicEditFunctions::SetVertexPosition(
				mesh, i, normal * newLength, bIsValidVertex);
		}
	}

	/*Planet->GetDynamicMeshComponent()->NotifyMeshUpdated();
	Planet->GetDynamicMeshComponent()->UpdateCollision();*/
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