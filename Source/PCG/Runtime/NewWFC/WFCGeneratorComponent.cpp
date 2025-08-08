#include "WFCGeneratorComponent.h"

#include "WFCVisualizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Async/Async.h"
#include "Kismet/KismetSystemLibrary.h"

UWFCGeneratorComponent::UWFCGeneratorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Configuration.GridSize = FIntVector(5, 5, 3);
	Configuration.GenerationMode = EWFCGenerationMode::GroundFirst;
	Configuration.MaxIterations = 1000;
	Configuration.bEnableBacktracking = true;
	Configuration.BacktrackingDepth = 10;
}

void UWFCGeneratorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bVisualizeTiles && GetOwner())
	{
		RootVisualization = NewObject<USceneComponent>(GetOwner());
		RootVisualization->SetupAttachment(GetOwner()->GetRootComponent());
		RootVisualization->RegisterComponent();
	}

	WFCCore = MakeUnique<FWFCCore>();

	if (TileSet)
	{
		InitializeWFCCore(Configuration);
	}
	if (bAutoGenerateOnBeginPlay)
	{
		StartGenerationWithCustomConfigAt(FVector(0, 0, 0), FRotator(0, 0, 0));
	}
}

void UWFCGeneratorComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ProcessNextRequest();

}

void UWFCGeneratorComponent::BeginDestroy()
{
	ClearQueue();
	Super::BeginDestroy();
}

void UWFCGeneratorComponent::InitializeWFCCore(const FWFCConfiguration& CustomConfig)
{
	if (bIsProcessingQueue)
	{
		UE_LOG(LogTemp, Warning, TEXT("WFCGenerator: Queue is being processed"));
		return;
	}

	if (!TileSet)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCGenerator: No TileSet assigned"));
		return;
	}

	CompleteTileSet = NewObject<UWFCTileSet>();

	UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Starting generation with grid size %s"),
	       *CustomConfig.GridSize.ToString());

	Configuration = CustomConfig;

	if (!WFCCore->Initialize(TileSet, Configuration))
	{
		UE_LOG(LogTemp, Error, TEXT("WFCGenerator: Failed to initialize WFC core"));
		return;
	}

	if (PreProcessCache)
	{
		WFCCore->SetPreProcessCache(PreProcessCache);
		UE_LOG(LogTemp, Log, TEXT("WFCGenerator: PreProcess cache set"));
	}

	WFCCore->OnStatusUpdate.BindUObject(this, &UWFCGeneratorComponent::OnWFCStatusUpdate);
}

void UWFCGeneratorComponent::StartGeneration()
{
	StartGenerationWithCustomConfig(Configuration);
}

void UWFCGeneratorComponent::StartGenerationWithCustomConfig(const FWFCConfiguration& CustomConfig)
{
	if (bIsProcessingQueue)
	{
		UE_LOG(LogTemp, Warning, TEXT("WFCGenerator: Queue is being processed"));
		return;
	}

	if (!TileSet)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCGenerator: No TileSet assigned"));
		return;
	}

	CompleteTileSet = NewObject<UWFCTileSet>();

	for (int i = 0; i < TileSet->Tiles.Num(); i++)
	{
		CompleteTileSet->Tiles.Add(TileSet->Tiles[i]);
	}
	for (int i = 0; i < TileSet->SocketDefinitions.Num(); i++)
	{
		CompleteTileSet->SocketDefinitions.Add(TileSet->SocketDefinitions[i]);
	}
	CompleteTileSet->DefaultConfiguration = TileSet->DefaultConfiguration;

	UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Starting generation with grid size %s"),
	       *CustomConfig.GridSize.ToString());

	Configuration = CustomConfig;

	ClearVisualization();

	if (!WFCCore->Initialize(TileSet, Configuration))
	{
		UE_LOG(LogTemp, Error, TEXT("WFCGenerator: Failed to initialize WFC core"));
		return;
	}

	if (bUseAsyncGeneration)
	{
		ExecuteGenerationAsync();
	}
	else
	{
		ExecuteGeneration();
	}
}

void UWFCGeneratorComponent::StartGenerationWithCustomGridSizeAt(FVector Location, FRotator Rotation, FIntVector GridSize)
{
	FGenerationRequest Request(Location, Rotation, 0, GridSize);
	PendingRequests.Add(Request);
}

void UWFCGeneratorComponent::StartGenerationWithCustomConfigAt(FVector Location, FRotator Rotation)
{
	FGenerationRequest Request(Location, Rotation, 0, Configuration.GridSize);
	PendingRequests.Add(Request);
}

int UWFCGeneratorComponent::ExecuteGenerationAsyncAt(FVector Location, FRotator Rotation)
{
	QueueGeneration(Location, Rotation);

	return 0;
}

int UWFCGeneratorComponent::QueueGeneration(FVector Location, FRotator Rotation)
{


	return 0;
}

void UWFCGeneratorComponent::ProcessNextRequest()
{
	FGenerationRequest Request;
	if (bIsProcessingQueue)
	{
		return;
	}

	if (PendingRequests.Num() == 0)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCGenerator: Queue is empty, stopping processing"));
		return;
	}
	Request = PendingRequests.Pop();

	bIsProcessingQueue = true;

	if (!WFCCore)
	{
		return;
	}

	Configuration.GridSize = Request.GridSize;
	WFCCore->UpdateGrid(Configuration);
	
	if (bUseAsyncGeneration)
	{
		Async(EAsyncExecution::TaskGraph, [this, Request]()
		{
			FWFCGenerationResult Result;
			
			UE_LOG(LogTemp, Warning, TEXT("GRID SIZE: %s"), *Configuration.GridSize.ToString());
			Result = WFCCore->Generate();


			AsyncTask(ENamedThreads::GameThread, [this, Request, Result]()
			{
				OnGenerationFinished(Result, Request.Location, Request.Rotation);
			});
			
		});
	}
	else
	{
		FWFCGenerationResult Result = WFCCore->Generate();
		OnGenerationFinished(Result, Request.Location, Request.Rotation);
		ProcessNextRequest();
	}
}

void UWFCGeneratorComponent::StopGeneration()
{
	bIsProcessingQueue = false;
	UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Generation stopped"));

	if (GenerationFuture.IsValid())
	{
		GenerationFuture.Wait();
	}
}

void UWFCGeneratorComponent::ClearGeneration()
{
	StopGeneration();
	ClearQueue();
	ClearVisualization();

	if (WFCCore)
	{
		WFCCore->Reset();
	}

	LastResult = FWFCGenerationResult();
	UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Generation cleared"));
}

void UWFCGeneratorComponent::ClearQueue()
{
	FScopeLock Lock(&QueueLock);
	int32 ClearedCount = PendingRequests.Num();
	PendingRequests.Empty();

	if (ClearedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Cleared %d requests from queue"), ClearedCount);
	}
}

int32 UWFCGeneratorComponent::GetQueueSize() const
{
	FScopeLock Lock(&QueueLock);
	return PendingRequests.Num();
}

TArray<FGenerationRequest> UWFCGeneratorComponent::GetQueuedRequests() const
{
	FScopeLock Lock(&QueueLock);
	return PendingRequests;
}

void UWFCGeneratorComponent::SetTileSet(UWFCTileSet* NewTileSet)
{
	if (bIsProcessingQueue)
	{
		UE_LOG(LogTemp, Warning, TEXT("WFCGenerator: Cannot change TileSet during queue processing"));
		return;
	}

	TileSet = NewTileSet;
	UE_LOG(LogTemp, Log, TEXT("WFCGenerator: TileSet updated"));
}

void UWFCGeneratorComponent::AddConstraint(const FWFCGenerationConstraint& Constraint)
{
	Configuration.Constraints.Add(Constraint);
	UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Added constraint: %s"), *Constraint.ConstraintName);
}

void UWFCGeneratorComponent::ClearConstraints()
{
	Configuration.Constraints.Empty();
	UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Cleared all constraints"));
}

void UWFCGeneratorComponent::SetVisualizationEnabled(bool bEnabled)
{
	bVisualizeTiles = bEnabled;

	if (!bEnabled)
	{
		ClearVisualization();
	}
	else if (LastResult.bSuccess)
	{
		RefreshVisualization();
	}
}

void UWFCGeneratorComponent::RefreshVisualization()
{
	if (bVisualizeTiles && LastResult.bSuccess)
	{
		ClearVisualization();
		CreateVisualization(LastResult);
	}
}

void UWFCGeneratorComponent::NextCollapseStep()
{
	if (LastCollapseHistory.Num() == 0) return;
	CurCollapseHistoryStep++;
	if (CurCollapseHistoryStep > LastCollapseHistory.Num() - 1)
	{
		CurCollapseHistoryStep = LastCollapseHistory.Num() - 1;
	}
	if (CurCollapseHistoryStep < 0) return;

	FWFCCoordinate Coord = LastCollapseHistory[CurCollapseHistoryStep];
	SpawnedActors[Coord]->GetComponentByClass<UStaticMeshComponent>()->SetVisibility(true);
	UKismetSystemLibrary::DrawDebugSphere(GetWorld(), SpawnedActors[Coord]->GetActorLocation(), 50, 12, FColor::Green,
	                                      3, 1);
}

void UWFCGeneratorComponent::PrevCollapseStep()
{
	if (LastCollapseHistory.Num() == 0) return;
	if (CurCollapseHistoryStep > 0 && CurCollapseHistoryStep <= LastCollapseHistory.Num() - 1)
	{
		FWFCCoordinate Coord = LastCollapseHistory[CurCollapseHistoryStep];
		SpawnedActors[Coord]->GetComponentByClass<UStaticMeshComponent>()->SetVisibility(false);
		UKismetSystemLibrary::DrawDebugSphere(GetWorld(), SpawnedActors[Coord]->GetActorLocation(), 50, 12, FColor::Red,
		                                      3, 1);
	}

	CurCollapseHistoryStep--;
	if (CurCollapseHistoryStep < 0)
	{
		CurCollapseHistoryStep = -1;
	}
}

void UWFCGeneratorComponent::SetGridSize(int X, int Y)
{
	Configuration.GridSize = FIntVector(X, Y, Configuration.GridSize.Z);
}

void UWFCGeneratorComponent::ExecuteGeneration()
{
	if (!WFCCore)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCGenerator: WFC Core not initialized"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Executing synchronous generation"));

	FWFCGenerationResult Result = WFCCore->Generate();
	OnGenerationFinished(Result);
}

void UWFCGeneratorComponent::ExecuteGenerationAsync()
{
	UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Executing asynchronous generation"));

	GenerationFuture = Async(EAsyncExecution::ThreadPool, [this]() -> FWFCGenerationResult
	{
		return WFCCore->Generate();
	});

	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		if (GenerationFuture.IsValid())
		{
			FWFCGenerationResult Result = GenerationFuture.Get();
			OnGenerationFinished(Result);
		}
	});

}

void UWFCGeneratorComponent::ExecuteGenerationAt(FVector Location, FRotator Rotation)
{
	if (!WFCCore)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCGenerator: WFC Core not initialized"));
		return;
	}
	
	FWFCGenerationResult Result = WFCCore->Generate();
	OnGenerationFinished(Result, Location, Rotation);
}

void UWFCGeneratorComponent::OnGenerationFinished(const FWFCGenerationResult& Result)
{
	LastResult = Result;

	if (bVisualizeTiles && Result.bSuccess)
	{
		CreateVisualization(Result);
	}

	OnGenerationComplete.Broadcast(Result);
}

void UWFCGeneratorComponent::OnGenerationFinished(const FWFCGenerationResult& Result, FVector Location, FRotator Rotation)
{
	LastResult = Result;
	LastCollapseHistory = WFCCore->GetCollapseHistory();
	CurCollapseHistoryStep = LastCollapseHistory.Num() - 1;
	
	if (bVisualizeTiles && Result.bSuccess)
	{
		if (!bUseAsyncGeneration)
			CreateVisualizationAt(Result, Location, Rotation);
		else
		{
			CreateVisualizationAtByFrame(Result, Location, Rotation);
		}
	}
	
	OnGenerationComplete.Broadcast(Result);
	
}

void UWFCGeneratorComponent::CreateVisualization(const FWFCGenerationResult& Result)
{
	if (!GetOwner() || !TileSet)
	{
		return;
	}

	int32 CreatedCount = 0;
	for (const auto& [Coord, TileIndex] : Result.TileAssignments)
	{
		if (AActor* TileActor = SpawnTileActor(Coord, TileIndex))
		{
			SpawnedActors.Add(Coord, TileActor);
			OnTileGenerated.Broadcast(Coord, TileIndex);
			CreatedCount++;
		}
	}
}

USceneComponent* UWFCGeneratorComponent::CreateVisualizationAt(const FWFCGenerationResult& Result, FVector Location,
                                                               FRotator Rotation)
{
	USceneComponent* ParentComp = NewObject<USceneComponent>(GetOwner());
	ParentComp->RegisterComponent();
	SpawnedTileParents.Add(ParentComp);
	if (!GetOwner() || !TileSet)
	{
		return ParentComp;
	}

	int32 CreatedCount = 0;
	for (const auto& [Coord, TileIndex] : Result.TileAssignments)
	{
		if (AActor* TileActor = SpawnTileActor(Coord, TileIndex))
		{
			SpawnedActors.Add(Coord, TileActor);
			TileActor->AttachToComponent(ParentComp, FAttachmentTransformRules::KeepRelativeTransform);
			OnTileGenerated.Broadcast(Coord, TileIndex);
			CreatedCount++;
		}
	}
	ParentComp->SetWorldLocation(Location);
	ParentComp->SetWorldRotation(Rotation);
	return ParentComp;
}

USceneComponent* UWFCGeneratorComponent::CreateVisualizationAtByFrame(const FWFCGenerationResult& Result,
                                                                      FVector Location, FRotator Rotation)
{
	FWFCVisualizationData VisualizationData;
	VisualizationData.ParentLocation = Location;
	VisualizationData.ParentRotation = Rotation;
	VisualizationData.bShowEmptyTiles = Configuration.bShowEmptyTiles;


	int32 CreatedCount = 0;
	for (const auto& [Coord, TileIndex] : Result.TileAssignments)
	{
		FWFCVisualizationTile Tile;
		FWFCTileDefinition TileDef = TileSet->GetTile(TileIndex);
		FVector WorldPosition = CoordinateToWorldPosition(Coord);
		FRotator WorldRotation = TileDef.BaseRotation;
		Tile.TileName = TileDef.TileName;
		Tile.Location = WorldPosition;
		Tile.Rotation = WorldRotation;
		Tile.StaticMesh = TileDef.Mesh;
		Tile.Material = TileDef.Material;
		Tile.Category = TileDef.Category;
		VisualizationData.Tiles.Add(Tile);
	}

	AWFCVisualizer* Visualizer = GetWorld()->SpawnActor<AWFCVisualizer>();
	Visualizer->SetActorLocation(FVector::ZeroVector);
	Visualizer->StartVisualization(GenerationActorPerFrame, VisualizationData);
	Visualizer->OnVisualizationComplete.AddDynamic(this, &UWFCGeneratorComponent::OnVisualizationComplete);
	return nullptr;
}

void UWFCGeneratorComponent::ClearVisualization()
{
	for (auto& [Coord, Actor] : SpawnedActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}

	SpawnedActors.Empty();
}

AActor* UWFCGeneratorComponent::SpawnTileActor(const FWFCCoordinate& Position, int32 TileIndex)
{
	if (!GetOwner() || !TileSet)
	{
		return nullptr;
	}

	FWFCTileDefinition TileDef = TileSet->GetTile(TileIndex);
	if (!TileDef.Mesh)
	{
		return nullptr;
	}

	FVector WorldPosition = CoordinateToWorldPosition(Position);
	FRotator WorldRotation = TileDef.BaseRotation;

	UWorld* World = GetOwner()->GetWorld();
	AActor* TileActor = World->SpawnActor<AActor>();

	if (!TileActor)
	{
		return nullptr;
	}


	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(TileActor);
	MeshComponent->RegisterComponent();
	MeshComponent->SetStaticMesh(TileDef.Mesh);

	if (TileDef.Material)
	{
		MeshComponent->SetMaterial(0, TileDef.Material);
	}

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

	TileActor->SetRootComponent(MeshComponent);


	if (RootVisualization)
	{
		TileActor->AttachToComponent(RootVisualization,
		                             FAttachmentTransformRules::KeepRelativeTransform);
	}
	TileActor->SetActorLocation(WorldPosition);
	TileActor->SetActorRotation(WorldRotation);
	return TileActor;
}

FVector UWFCGeneratorComponent::CoordinateToWorldPosition(const FWFCCoordinate& Coord) const
{
	FVector BasePosition = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;

	FVector CellPosition = FVector(
		Coord.X * CellSize,
		Coord.Y * CellSize,
		Coord.Z * CellSize
	) - FVector(CellSize * Configuration.GridSize.X / 2.f, CellSize * Configuration.GridSize.Y / 2.f, 0);

	return BasePosition + CellPosition;
}

FVector UWFCGeneratorComponent::CoordinateToLocalPosition(const FWFCCoordinate& Coord) const
{
	FVector BasePosition = FVector::ZeroVector;

	FVector CellPosition = FVector(
		Coord.X * CellSize,
		Coord.Y * CellSize,
		Coord.Z * CellSize
	) - FVector(CellSize * Configuration.GridSize.X / 2.f, CellSize * Configuration.GridSize.Y / 2.f, 0);

	return BasePosition + CellPosition;
}

void UWFCGeneratorComponent::OnWFCStatusUpdate(FWFCCoordinate Coord, int32 Tile)
{
	//SpawnTileActor(Coord, Tile);
}

void UWFCGeneratorComponent::OnVisualizationComplete(AWFCVisualizer* Visualizer)
{
	bIsProcessingQueue = false;
}


void UWFCGeneratorComponent::SetPreProcessCache(UWFCPreProcessCache* InCache)
{
	PreProcessCache = InCache;
	if (WFCCore)
	{
		WFCCore->SetPreProcessCache(PreProcessCache);
	}
}
