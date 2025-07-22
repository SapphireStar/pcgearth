#include "TerrainBuildAbility.h"
#include "EngineUtils.h"
#include "Camera/CameraComponent.h"
#include "GeometryScript/GeometryScriptSelectionTypes.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PCG/Runtime/PCGGameMode.h"
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

	if (!GridSelection && GetWorld())
	{
		for (TActorIterator<AGridSelectionManager> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			GridSelection = *ActorItr;
			break;
		}
	}
}

void UTerrainBuildAbility::OnInitializeAbility()
{
	Super::OnInitializeAbility();
	PlayerData = Cast<APCGGameMode>(GetWorld()->GetAuthGameMode())->PlayerData;
	UE_LOG(LogTemp, Log, TEXT("TerrainBuildAbility initialized"));
}

void UTerrainBuildAbility::OnActivateAbility()
{
	Super::OnActivateAbility();
	bIsGridSlectionStarted = false;
	UE_LOG(LogTemp, Log, TEXT("TerrainBuildAbility activated - Ready to build!"));
}

void UTerrainBuildAbility::OnTickAbility()
{
	Super::OnTickAbility();
}

void UTerrainBuildAbility::OnDeactivateAbility()
{
	Super::OnDeactivateAbility();
	GridSelection->ShutDownGridSelection();
	bIsGridSlectionStarted = false;
	DeselectPlanet();
	UE_LOG(LogTemp, Log, TEXT("TerrainBuildAbility deactivated"));
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

	if (!GridSelection)
	{
		UE_LOG(LogTemp, Warning, TEXT("Grid Selection is invalid"));
		return;
	}

	if (!bIsGridSlectionStarted)
	{
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
				SelectPlanet(planet, HitResult);
				GridSelection->StartGridSelection(HitResult.ImpactPoint,
				                                  FindNormalOnPlanet(HitResult.ImpactPoint,
				                                                     planet->GetActorLocation()));
				bIsGridSlectionStarted = true;
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
	else if (Planet)
	{
		bIsGridSlectionStarted = false;
		FBox GridBounds = GridSelection->EndGridSelection();
		FVector GeneratePos = GridBounds.GetCenter();
		ProcessTerrainBuild(Planet, LastHitResult, GridBounds);
		//WFCGeneratorComponent->StartGenerationWithCustomConfigAt(GeneratePos, GridSelection->GetGridRotation());
		DeselectPlanet();
	}
	else
	{
		DeselectPlanet();
		bIsGridSlectionStarted = false;
		UE_LOG(LogTemp, Warning, TEXT("No valid planet found"));
	}
}

void UTerrainBuildAbility::ProcessTerrainBuild(AGeometryPlanet* Planet, const FHitResult& HitResult, FBox GridBounds)
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
		SpawnBuilding(Planet, HitResult, GridBounds);

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

void UTerrainBuildAbility::SpawnBuilding(AGeometryPlanet* Planet, const FHitResult& HitResult, FBox GridBounds)
{
	FVector normal = HitResult.ImpactPoint - Planet->GetActorLocation();
	normal.Normalize();
	FRotator rotation = UKismetMathLibrary::MakeRotFromZ(normal);

	if (WFCGeneratorComponent)
	{
		if (TryConsumeWood())
		{
			CalculateWFCGridSize(GridBounds);
			WFCGeneratorComponent->StartGenerationWithCustomConfigAt(GridBounds.GetCenter(),
			                                                         GridSelection->GetGridRotation());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No WFC generator available for building construction"));
	}
}

bool UTerrainBuildAbility::TryConsumeWood()
{
	if (WFCGeneratorComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Try consume wood"));
		int volume = WFCGeneratorComponent->Configuration.GridSize.X * WFCGeneratorComponent->Configuration.GridSize.Y *
			WFCGeneratorComponent->Configuration.GridSize.Z;
		if (PlayerData->GetPlayerWoodValue() >= volume)
		{
			UE_LOG(LogTemp, Warning, TEXT("%d Wood consumed"), volume);
			PlayerData->ChangePlayerWoodValue(PlayerData->GetPlayerWoodValue() - volume);
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Player wood not available"), volume);
		}
	}
	return false;
}

int UTerrainBuildAbility::FindVertex(const FVector& Target, UDynamicMeshComponent* DynamicMeshComp,
                                     TArray<int32> VertexID)
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

FRotator UTerrainBuildAbility::FindNormalOnPlanet(FVector ImpactPosition, FVector PlanetPosition)
{
	FVector normal = (ImpactPosition - PlanetPosition).GetSafeNormal();
	return UKismetMathLibrary::MakeRotFromZ(normal);
}

void UTerrainBuildAbility::CalculateWFCGridSize(FBox GridBounds)
{
	int SizeX = static_cast<int>((GridBounds.Max.X - GridBounds.Min.X)/GridSelection->GetGridSize());
	int SizeY = static_cast<int>((GridBounds.Max.Y - GridBounds.Min.Y)/GridSelection->GetGridSize());
	WFCGeneratorComponent->Configuration.GridSize = FIntVector(SizeX, SizeY, WFCGeneratorComponent->Configuration.GridSize.Z);
}

void UTerrainBuildAbility::SelectPlanet(AGeometryPlanet* Planet, FHitResult& HitResult)
{
	this->Planet = Planet;
	LastHitResult = HitResult;
}

void UTerrainBuildAbility::DeselectPlanet()
{
	this->Planet = nullptr;
	LastHitResult = FHitResult();
}
