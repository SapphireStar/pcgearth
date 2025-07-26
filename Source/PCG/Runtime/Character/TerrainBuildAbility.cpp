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
			if (AGeometryPlanetActor* planet = Cast<AGeometryPlanetActor>(HitResult.GetActor()))
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
		FBox GridBounds = GridSelection->PeekGridSelection();
		AMineSphere* MineSphere = CheckIsOnMineSphere(GridBounds);
		if (ValidateGridBounds(GridBounds) && MineSphere)
		{
			if (ProcessTerrainBuild(Planet, LastHitResult, GridBounds, MineSphere))
			{
				bIsGridSlectionStarted = false;
				GridSelection->EndGridSelection();
				DeselectPlanet();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Process Terrain Build failed"));
			}
			
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Grid selection is invalid"));
		}
	}
	else
	{
		DeselectPlanet();
		bIsGridSlectionStarted = false;
		UE_LOG(LogTemp, Warning, TEXT("No valid planet found"));
	}
}

bool UTerrainBuildAbility::ProcessTerrainBuild(AGeometryPlanetActor* Planet, const FHitResult& HitResult, FBox GridBounds, AMineSphere* MineSphere)
{
	if (!Planet)
	{
		return false;
	}

	FVector ImpactRelativePoint = HitResult.ImpactPoint - Planet->GetActorLocation();
	FGeometryScriptMeshSelection selection;

	UGeometryScriptLibrary_MeshSelectionFunctions::SelectMeshElementsInSphere(
		Planet->GetDynamicMeshComponent()->GetDynamicMesh(),
		selection,
		GridBounds.GetCenter() - Planet->GetActorLocation(),
		FVector::DistXY(GridBounds.Max, GridBounds.Min)/2.f,
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
		

		if (SpawnBuilding(Planet, HitResult, GridBounds,MineSphere))
		{
			FlattenTerrain(Planet, indicesout, GridBounds);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn building"));
			return false;
		}

		UE_LOG(LogTemp, Log, TEXT("Building constructed successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No suitable terrain found for building"));
		return false;
	}
	return true;
}

void UTerrainBuildAbility::FlattenTerrain(AGeometryPlanetActor* Planet, const TArray<int32>& VertexIndices, FBox GridBounds)
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

bool UTerrainBuildAbility::SpawnBuilding(AGeometryPlanetActor* Planet, const FHitResult& HitResult, FBox GridBounds, AMineSphere* MineSphere)
{
	FVector normal = HitResult.ImpactPoint - Planet->GetActorLocation();
	normal.Normalize();
	FRotator rotation = UKismetMathLibrary::MakeRotFromZ(normal);

	if (WFCGeneratorComponent)
	{
		int volume = 0;
		if (TryConsumeWood(volume))
		{
			CalculateWFCGridSize(GridBounds);
			WFCGeneratorComponent->StartGenerationWithCustomConfigAt(GridBounds.GetCenter(),
			                                                         GridSelection->GetGridRotation());
			
			SpawnFactoryActor(GridBounds.GetCenter(), volume, MineSphere);
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to consume wood!"));
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No WFC generator available for building construction"));
		return false;
	}
}

void UTerrainBuildAbility::SpawnFactoryActor(FVector Position, int Volume, AMineSphere* MineSphere)
{
	AFactoryBuilding* Factory = GetWorld()->SpawnActor<AFactoryBuilding>();
	Factory->BuildFactoryAt(Position, Volume, MineSphere);
}

bool UTerrainBuildAbility::TryConsumeWood(int& outVolume)
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
			outVolume = volume;
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Player wood not available"));
		}
	}
	outVolume = 0;
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
	WFCGeneratorComponent->Configuration.GridSize = FIntVector(SizeY, SizeX, WFCGeneratorComponent->Configuration.GridSize.Z);
}

bool UTerrainBuildAbility::ValidateGridBounds(FBox GridBounds)
{
	if (GridBounds.Max.X <= GridBounds.Min.X ||
		GridBounds.Max.Y <= GridBounds.Min.Y)
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainBuildAbility: Grid size invalid"));
		return false;
	}

	if (GridBounds.Max.X - GridBounds.Min.X < GridSelection->GetGridSize()/2 ||
		GridBounds.Max.Y - GridBounds.Min.Y < GridSelection->GetGridSize()/2)
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainBuildAbility: Grid size too small"));
		return false;
	}

	return true;
}

AMineSphere* UTerrainBuildAbility::CheckIsOnMineSphere(FBox GridBounds)
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
    
	UClass* ActorClassFilter = AMineSphere::StaticClass();
	
	TArray<AActor*> ActorsToIgnore;
	TArray<AActor*> FoundActors;
	
	UKismetSystemLibrary::BoxOverlapActors(
		GetWorld(),
		GridBounds.GetCenter(),
		GridBounds.GetExtent(),
		ObjectTypes,
		ActorClassFilter,
		ActorsToIgnore,
		FoundActors
	);
	if (FoundActors.Num() > 0)
	{
		return Cast<AMineSphere>(FoundActors[0]);
	}
	return nullptr;
}

FVector UTerrainBuildAbility::CalculateReferencePoint(UDynamicMeshComponent* Mesh, const TArray<int32>& VertexIndices,
	bool bUseLowest)
{
	if (bUseLowest) {
		int32 LowestVertexID = FindLowestVertex(Mesh, VertexIndices);
		bool bIsValid;
		return UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(Mesh->GetDynamicMesh(), LowestVertexID, bIsValid);
	} else {
		FVector Sum = FVector::ZeroVector;
		int32 ValidCount = 0;
		return Sum / ValidCount;
	}
}

void UTerrainBuildAbility::SelectPlanet(AGeometryPlanetActor* Planet, FHitResult& HitResult)
{
	this->Planet = Planet;
	LastHitResult = HitResult;
}

void UTerrainBuildAbility::DeselectPlanet()
{
	this->Planet = nullptr;
	LastHitResult = FHitResult();
}
