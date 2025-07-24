#include "TestWFCAbility.h"
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
#include "TerrainBuildAbility.h"

UTestWFCAbility::UTestWFCAbility()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTestWFCAbility::BeginPlay()
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

void UTestWFCAbility::OnInitializeAbility()
{
	Super::OnInitializeAbility();
	PlayerData = Cast<APCGGameMode>(GetWorld()->GetAuthGameMode())->PlayerData;
	UE_LOG(LogTemp, Log, TEXT("TestWFCAbility initialized"));
}

void UTestWFCAbility::OnActivateAbility()
{
	Super::OnActivateAbility();
	bIsGridSlectionStarted = false;
	UE_LOG(LogTemp, Log, TEXT("TestWFCAbility activated - Ready to build!"));
}

void UTestWFCAbility::OnTickAbility()
{
	Super::OnTickAbility();
}

void UTestWFCAbility::OnDeactivateAbility()
{
	Super::OnDeactivateAbility();
	GridSelection->ShutDownGridSelection();
	bIsGridSlectionStarted = false;
	DeselectPlanet();
	UE_LOG(LogTemp, Log, TEXT("TestWFCAbility deactivated"));
}

void UTestWFCAbility::OnStartUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
	Super::OnStartUseAbility(TraceStartComp, Camera);
}

void UTestWFCAbility::OnKeepUsingAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
	Super::OnKeepUsingAbility(TraceStartComp, Camera);
}

void UTestWFCAbility::OnCompleteUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
	Super::OnCompleteUseAbility(TraceStartComp, Camera);

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
				WFCGeneratorComponent->StartGenerationWithCustomConfigAt(HitResult.Location, UKismetMathLibrary::MakeRotFromZ(HitResult.ImpactNormal));
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

bool UTestWFCAbility::ProcessTerrainBuild(AGeometryPlanet* Planet, const FHitResult& HitResult, FBox GridBounds, AMineSphere* MineSphere)
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
		FVector(GridBounds.GetCenter().X, GridBounds.GetCenter().Y, HitResult.ImpactPoint.Z) - Planet->GetActorLocation(),
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

void UTestWFCAbility::FlattenTerrain(AGeometryPlanet* Planet, const TArray<int32>& VertexIndices, FBox GridBounds)
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

bool UTestWFCAbility::SpawnBuilding(AGeometryPlanet* Planet, const FHitResult& HitResult, FBox GridBounds, AMineSphere* MineSphere)
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

void UTestWFCAbility::SpawnFactoryActor(FVector Position, int Volume, AMineSphere* MineSphere)
{
	AFactoryBuilding* Factory = GetWorld()->SpawnActor<AFactoryBuilding>();
	Factory->BuildFactoryAt(Position, Volume, MineSphere);
}

bool UTestWFCAbility::TryConsumeWood(int& outVolume)
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

int UTestWFCAbility::FindVertex(const FVector& Target, UDynamicMeshComponent* DynamicMeshComp,
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

int UTestWFCAbility::FindLowestVertex(UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID)
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

FRotator UTestWFCAbility::FindNormalOnPlanet(FVector ImpactPosition, FVector PlanetPosition)
{
	FVector normal = (ImpactPosition - PlanetPosition).GetSafeNormal();
	return UKismetMathLibrary::MakeRotFromZ(normal);
}

void UTestWFCAbility::CalculateWFCGridSize(FBox GridBounds)
{
	int SizeX = static_cast<int>((GridBounds.Max.X - GridBounds.Min.X)/GridSelection->GetGridSize());
	int SizeY = static_cast<int>((GridBounds.Max.Y - GridBounds.Min.Y)/GridSelection->GetGridSize());
	WFCGeneratorComponent->Configuration.GridSize = FIntVector(SizeY, SizeX, WFCGeneratorComponent->Configuration.GridSize.Z);
}

bool UTestWFCAbility::ValidateGridBounds(FBox GridBounds)
{
	if (GridBounds.Max.X <= GridBounds.Min.X ||
		GridBounds.Max.Y <= GridBounds.Min.Y)
	{
		UE_LOG(LogTemp, Error, TEXT("TestWFCAbility: Grid size invalid"));
		return false;
	}

	if (GridBounds.Max.X - GridBounds.Min.X < GridSelection->GetGridSize()/2 ||
		GridBounds.Max.Y - GridBounds.Min.Y < GridSelection->GetGridSize()/2)
	{
		UE_LOG(LogTemp, Error, TEXT("TestWFCAbility: Grid size too small"));
		return false;
	}

	return true;
}

AMineSphere* UTestWFCAbility::CheckIsOnMineSphere(FBox GridBounds)
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

void UTestWFCAbility::SelectPlanet(AGeometryPlanet* Planet, FHitResult& HitResult)
{
	this->Planet = Planet;
	LastHitResult = HitResult;
}

void UTestWFCAbility::DeselectPlanet()
{
	this->Planet = nullptr;
	LastHitResult = FHitResult();
}
