// TerrainBuildAbility.cpp

#include "TerrainBuildAbility.h"
#include "EngineUtils.h"
#include "Camera/CameraComponent.h"
#include "GeometryScript/GeometryScriptSelectionTypes.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PCG/Runtime/NewPlanet/GeometryPlanet.h"
#include "PCG/Runtime/WaveFunctionCollapse/WFCGenerator.h"
#include "PCG/Runtime/NewWFC/WFCGeneratorComponent.h"

UTerrainBuildAbility::UTerrainBuildAbility()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTerrainBuildAbility::BeginPlay()
{
	Super::BeginPlay();
	
	if (!WFCGeneratorComponent && GetWorld())
	{
		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (auto comp = ActorItr->GetComponentByClass<UWFCGeneratorComponent>())
			{
				WFCGeneratorComponent = comp;
				break;
			}
		}
	}
}

void UTerrainBuildAbility::OnInitializeAbility()
{
	Super::OnInitializeAbility();
	UE_LOG(LogTemp, Log, TEXT("TerrainBuildAbility initialized"));
}

void UTerrainBuildAbility::OnActivateAbility()
{
	Super::OnActivateAbility();
	UE_LOG(LogTemp, Log, TEXT("TerrainBuildAbility activated - Ready to build!"));
	
	// 激活时可以显示建造预览等
}

void UTerrainBuildAbility::OnTickAbility()
{
	Super::OnTickAbility();
	// 如果需要持续更新的逻辑，可以在这里实现
	// 例如：显示建造预览、更新UI等
}

void UTerrainBuildAbility::OnDeactivateAbility()
{
	Super::OnDeactivateAbility();
	UE_LOG(LogTemp, Log, TEXT("TerrainBuildAbility deactivated"));
	
	// 停用时清理预览等
}

void UTerrainBuildAbility::OnStartUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
	Super::OnStartUseAbility(TraceStartComp, Camera);
}

void UTerrainBuildAbility::OnKeepUsingAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
	Super::OnKeepUsingAbility(TraceStartComp, Camera);
}

void UTerrainBuildAbility::OnCompleteUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
	Super::OnCompleteUseAbility(TraceStartComp, Camera);
	if (!TraceStartComp || !Camera)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid components for building"));
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
		if (AGeometryPlanet* planet = Cast<AGeometryPlanet>(HitResult.GetActor()))
		{
			ProcessTerrainBuild(planet, HitResult);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit non-planet object, cannot build here"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid build target found"));
	}
}

void UTerrainBuildAbility::ProcessTerrainBuild(AGeometryPlanet* Planet, const FHitResult& HitResult)
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
		// 平整地形以便建造
		FlattenTerrain(Planet, indicesout);
		
		// 在平整的地形上生成建筑
		SpawnBuilding(Planet, HitResult);
		
		UE_LOG(LogTemp, Log, TEXT("Building constructed successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No suitable terrain found for building"));
	}
}

void UTerrainBuildAbility::FlattenTerrain(AGeometryPlanet* Planet, const TArray<int32>& VertexIndices)
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

	// 将选中区域的顶点平整到最低点的高度
	for (int i : VertexIndices)
	{
		if (i != LowestVertexID)
		{
			auto pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
				mesh, i, bIsValidVertex);
			FVector normal = pos.GetSafeNormal();
			
			UGeometryScriptLibrary_MeshBasicEditFunctions::SetVertexPosition(
				mesh, i, normal * lowestLength, bIsValidVertex);
		}
	}

	Planet->GetDynamicMeshComponent()->NotifyMeshUpdated();
	Planet->GetDynamicMeshComponent()->UpdateCollision();
}

void UTerrainBuildAbility::SpawnBuilding(AGeometryPlanet* Planet, const FHitResult& HitResult)
{
	// 计算建筑朝向（垂直于星球表面）
	FVector normal = HitResult.ImpactPoint - Planet->GetActorLocation();
	normal.Normalize();
	FRotator rotation = UKismetMathLibrary::MakeRotFromZ(normal);
	
	// 使用WFC生成器生成建筑
	if (WFCGeneratorComponent)
	{
		WFCGeneratorComponent->StartGenerationWithCustomConfigAt(HitResult.ImpactPoint, rotation);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No WFC generator available for building construction"));
	}
}

int UTerrainBuildAbility::FindVertex(const FVector& Target, UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID)
{
	auto DynamicMesh = DynamicMeshComp->GetDynamicMesh();
	bool bHasIdGroup;

	float min = FLT_MAX;
	int minID = 0;
	for (auto id : VertexID)
	{
		FVector pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(DynamicMesh, id, bHasIdGroup);
		if (FVector::Dist(pos, Target) < min)
		{
			min = FVector::Dist(pos, Target);
			minID = id;
		}
	}
	return minID;
}

int UTerrainBuildAbility::FindLowestVertex(UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID)
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