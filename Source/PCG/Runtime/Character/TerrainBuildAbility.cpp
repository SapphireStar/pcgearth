#include "TerrainBuildAbility.h"
#include "EngineUtils.h"
#include "PCG/Runtime/Factory/FactoryBuilding.h"
#include "ViewportInteractionTypes.h"
#include "Camera/CameraComponent.h"
#include "GeometryScript/GeometryScriptSelectionTypes.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PCG/Runtime/PCGGameMode.h"
#include "PCG/Runtime/Factory/FactoryCrafter.h"
#include "PCG/Runtime/NewPlanet/GeometryPlanetActor.h"
#include "PCG/Runtime/WaveFunctionCollapse/WFCGenerator.h"
#include "PCG/Runtime/NewWFC/WFCGeneratorComponent.h"

UTerrainBuildAbility::UTerrainBuildAbility()
{
	PrimaryComponentTick.bCanEverTick = false;
	FactorySphereMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FactorySphereMesh"));
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

	if (!FactoryManager && GetWorld())
	{
		for (TActorIterator<AFactoryManager>  ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			FactoryManager = *ActorItr;
			break;
		}
	}

	//FactorySphereMeshComponent->SetupAttachment(GetOwner()->GetRootComponent());
	InitializeMineSphere();
}

void UTerrainBuildAbility::OnInitializeAbility()
{
	Super::OnInitializeAbility();
	PlayerData = Cast<APCGGameMode>(GetWorld()->GetAuthGameMode())->PlayerData;
}

void UTerrainBuildAbility::OnActivateAbility()
{
	Super::OnActivateAbility();
	bIsGridSlectionStarted = false;
}

void UTerrainBuildAbility::OnTickAbility()
{
	Super::OnTickAbility();
	if (bIsGridSlectionStarted)
	{
		FBox GridBounds = GridSelection->PeekGridSelection();
		FVector TargetPos = GridBounds.GetCenter();
		if (CheckCanBuildFactory(TargetPos, 0, GridBounds) && CheckIsOnMineSphere(GridBounds))
		{
			ChangeFactorySphereColor(AvailableColor);
		}
		else
		{
			ChangeFactorySphereColor(UnavailableColor);
		}
		FactorySphereMeshComponent->SetWorldLocation(TargetPos);
	}
	
}

void UTerrainBuildAbility::OnDeactivateAbility()
{
	Super::OnDeactivateAbility();
	if (GridSelection)
		GridSelection->ShutDownGridSelection();
	bIsGridSlectionStarted = false;
	DeselectPlanet();
	FactorySphereMeshComponent->SetVisibility(false);
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

				if (bShouldCheckNearFactory)
				{
					FactorySphereMeshComponent->SetVisibility(true);
				}
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
			FIntVector GridSize = GridSelection->PeekGridSize();
			if (ProcessBuilding(Planet, LastHitResult, GridBounds, GridSize, MineSphere))
			{
				bIsGridSlectionStarted = false;
				GridSelection->EndGridSelection();
				DeselectPlanet();
				FactorySphereMeshComponent->SetVisibility(false);
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

void UTerrainBuildAbility::InitializeMineSphere()
{
	if (FactorySphereMesh && FactorySphereMaterial)
	{
		FactorySphereMeshComponent->SetStaticMesh(FactorySphereMesh);
		FactorySphereDynamicMaterial = UMaterialInstanceDynamic::Create(FactorySphereMaterial, this);
		FactorySphereMeshComponent->SetMaterial(0,  FactorySphereDynamicMaterial);
		float MeshRadius = FactorySphereMeshComponent->GetStaticMesh()->GetBoundingBox().GetExtent().X;
		float MeshScale =PlayerData->GetPlayerData().MiningFactoryInfo.FactoryRadius / MeshRadius;
		FactorySphereMeshComponent->SetWorldScale3D(FVector(MeshScale, MeshScale, MeshScale));
		FactorySphereMeshComponent->SetVisibility(false);
		FactorySphereMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

bool UTerrainBuildAbility::CheckCanBuildFactory(FVector Position, int Volume, FBox GridBounds)
{
	
	FGeometryScriptMeshSelection selection;

	UGeometryScriptLibrary_MeshSelectionFunctions::SelectMeshElementsInSphere(
		Planet->GetDynamicMeshComponent()->GetDynamicMesh(),
		selection,
		Position - Planet->GetActorLocation(),
		FVector::DistXY(GridBounds.Max, GridBounds.Min) / 2.f + 200.f,
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
		int LowestVertexID = FindLowestVertex(Planet->GetDynamicMeshComponent(), indicesout);
		bool bIsValidVertex;
		auto Mesh = Planet->GetDynamicMeshComponent()->GetDynamicMesh();

		auto lowestPos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
			Mesh, LowestVertexID, bIsValidVertex);
		
		FVector BuildingPos = GridBounds.GetCenter();
		FVector BuildingPosNormal = (BuildingPos - Planet->GetActorLocation()).GetSafeNormal();
		float LowestLength = lowestPos.Length();
		BuildingPos = Planet->GetActorLocation() + BuildingPosNormal * LowestLength - 50.f;

		float Radius = CalculateCollisionCheckRadius(GridBounds);
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));

		TArray<AActor*> ActorsToIgnore;
		TArray<AActor*> OverlappingActors;
		bool hit = UKismetSystemLibrary::SphereOverlapActors(
			GetWorld(),
			BuildingPos,
			Radius,
			ObjectTypes,
			nullptr,
			ActorsToIgnore,
			OverlappingActors);
		if (hit)
		{
			for (auto Actor : OverlappingActors)
			{
				if (Actor->IsA<AMiningBuilding>() || Actor->IsA<ACraftingBuilding>())
				{
					return false;
				}
			}
		}
	}
	else
	{
		float Radius = CalculateCollisionCheckRadius(GridBounds);
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));

		TArray<AActor*> ActorsToIgnore;
		TArray<AActor*> OverlappingActors;
		bool hit = UKismetSystemLibrary::SphereOverlapActors(
			GetWorld(),
			Position,
			Radius,
			ObjectTypes,
			nullptr,
			ActorsToIgnore,
			OverlappingActors);
		if (hit)
		{
			for (auto Actor : OverlappingActors)
			{
				if (Actor->IsA<AMiningBuilding>() || Actor->IsA<ACraftingBuilding>())
				{
					return false;
				}
			}
		}
	}
	

	return true;
}

float UTerrainBuildAbility::CalculateFactoryRadius(int Volume)
{
	return PlayerData->GetPlayerData().MiningFactoryInfo.FactoryRadius;
}

float UTerrainBuildAbility::CalculateCollisionCheckRadius(FBox GridBounds)
{
	return GridBounds.GetExtent().Length()/2.0f;
}

void UTerrainBuildAbility::ChangeFactorySphereColor(FLinearColor NewColor)
{
	if (bShouldCheckNearFactory)
		FactorySphereDynamicMaterial->SetVectorParameterValue(FName("Color"), NewColor);
}

bool UTerrainBuildAbility::ProcessBuilding(AGeometryPlanetActor* Planet, const FHitResult& HitResult, FBox GridBounds,
                                           FIntVector GridSize, AMineSphere* MineSphere)
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
		FVector::DistXY(GridBounds.Max, GridBounds.Min) / 2.f + 200.f,
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
		int LowestVertexID = FindLowestVertex(Planet->GetDynamicMeshComponent(), indicesout);
		bool bIsValidVertex;
		auto Mesh = Planet->GetDynamicMeshComponent()->GetDynamicMesh();

		auto lowestPos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
			Mesh, LowestVertexID, bIsValidVertex);
		if (SpawnBuilding(Planet, HitResult, GridBounds, GridSize, MineSphere, lowestPos))
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

void UTerrainBuildAbility::FlattenTerrain(AGeometryPlanetActor* Planet, const TArray<int32>& VertexIndices,
                                          FBox GridBounds)
{
	if (!Planet || VertexIndices.Num() == 0)
	{
		return;
	}

	int LowestVertexID = FindLowestVertex(Planet->GetDynamicMeshComponent(), VertexIndices);
	bool bIsValidVertex;
	auto Mesh = Planet->GetDynamicMeshComponent()->GetDynamicMesh();

	auto lowestPos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
		Mesh, LowestVertexID, bIsValidVertex);
	FVector PlaneNormal = (GridBounds.GetCenter() - Planet->GetActorLocation()).GetSafeNormal();

	for (int32 VertexID : VertexIndices)
	{
		bool bIsValid;
		FVector CurrentPos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(Mesh, VertexID, bIsValid);

		if (bIsValid)
		{
			FVector ToVertex = CurrentPos - lowestPos;
			float DistanceToPlane = FVector::DotProduct(ToVertex, PlaneNormal);
			FVector ProjectedPos = CurrentPos - (DistanceToPlane * PlaneNormal);

			UGeometryScriptLibrary_MeshBasicEditFunctions::SetVertexPosition(Mesh, VertexID, ProjectedPos, bIsValid);
		}
	}

	Planet->GetDynamicMeshComponent()->NotifyMeshUpdated();
	Planet->GetDynamicMeshComponent()->UpdateCollision();
}

//这里LowestVertexPos是相对顶点坐标，不是世界坐标
bool UTerrainBuildAbility::SpawnBuilding(AGeometryPlanetActor* Planet, const FHitResult& HitResult, FBox GridBounds,
                                         FIntVector GridSize, AMineSphere* MineSphere, FVector LowestVertexPos)
{
	FVector BuildingPos = GridBounds.GetCenter();
	FVector BuildingPosNormal = (BuildingPos - Planet->GetActorLocation()).GetSafeNormal();
	float LowestLength = LowestVertexPos.Length();
	BuildingPos = Planet->GetActorLocation() + BuildingPosNormal * LowestLength - 50.f;

	if (WFCGeneratorComponent)
	{
		//TODO:根据GridSize查找应有的Z高度
		int volume = 0;
		FIntVector FactorySize = GridSize;
		WFCGeneratorComponent->Configuration.GridSize = FIntVector(FactorySize.Y, FactorySize.X,
		                                                           WFCGeneratorComponent->Configuration.GridSize.Z);
		
		volume = WFCGeneratorComponent->Configuration.GridSize.X * WFCGeneratorComponent->Configuration.GridSize.Y *
	WFCGeneratorComponent->Configuration.GridSize.Z;

		//检查工厂附近是否有其他工厂，如果有，则不允许建造
		if (!CheckCanBuildFactory(BuildingPos, volume, GridBounds))
		{
			UE_LOG(LogTemp, Warning, TEXT("Too close to other factory!"));
			return false;
		}
		
		if (TryConsumeWood(volume))
		{
			WFCGeneratorComponent->StartGenerationWithCustomConfigAt(BuildingPos,
			                                                         GridSelection->GetGridRotation());
			SpawnFactoryActor(BuildingPos, volume, MineSphere, CalculateFactoryRadius(volume));
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

void UTerrainBuildAbility::SpawnFactoryActor(FVector Position, int Volume, AMineSphere* MineSphere, float Radius)
{
	FactoryManager->BuildMiningFactoryAt(Position, Volume, MineSphere, PlayerData->GetPlayerData().MiningFactoryInfo);
}

bool UTerrainBuildAbility::TryConsumeWood(int& outVolume)
{
	if (WFCGeneratorComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Try consume wood"));
		int volume = WFCGeneratorComponent->Configuration.GridSize.X * WFCGeneratorComponent->Configuration.GridSize.Y *
			WFCGeneratorComponent->Configuration.GridSize.Z;
		if (PlayerData->GetPlayerResourceValue(EFactoryResource::EFR_Wood) >= volume)
		{
			UE_LOG(LogTemp, Warning, TEXT("%d Wood consumed"), volume);
			PlayerData->ChangePlayerResourceValue(EFactoryResource::EFR_Wood, PlayerData->GetPlayerResourceValue(EFactoryResource::EFR_Wood) - volume);
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

	float min = FLT_MAX;
	int minID = 0;
	for (auto id : VertexID)
	{
		FVector pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(DynamicMesh, id, bHasIdGroup);
		float dist = pos.Length();
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

FIntVector UTerrainBuildAbility::CalculateWFCGridSize(FBox GridBounds)
{
	int SizeX = UKismetMathLibrary::Round((GridBounds.Max.X - GridBounds.Min.X) / GridSelection->GetGridSize());
	int SizeY = UKismetMathLibrary::Round((GridBounds.Max.Y - GridBounds.Min.Y) / GridSelection->GetGridSize());
	int SizeZ = UKismetMathLibrary::Round((GridBounds.Max.Z - GridBounds.Min.Z) / GridSelection->GetGridSize());
	return FIntVector(SizeX, SizeY, SizeZ);
}

bool UTerrainBuildAbility::ValidateGridBounds(FBox GridBounds)
{
	if (GridBounds.Max.X <= GridBounds.Min.X ||
		GridBounds.Max.Y <= GridBounds.Min.Y)
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainBuildAbility: Grid size invalid"));
		return false;
	}

	if (GridBounds.Max.X - GridBounds.Min.X < GridSelection->GetGridSize() / 2 ||
		GridBounds.Max.Y - GridBounds.Min.Y < GridSelection->GetGridSize() / 2)
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
