// Fill out your copyright notice in the Description page of Project Settings.


#include "WFCVisualizer.h"

#include "WFCTileSet.h"
#include "PCG/Runtime/PCGGameMode.h"


// Sets default values
AWFCVisualizer::AWFCVisualizer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>("RootSceneComponent");
	RootComponent = RootSceneComponent;

	
	
}

// Called when the game starts or when spawned
void AWFCVisualizer::BeginPlay()
{
	Super::BeginPlay();
	auto PlayerData = Cast<APCGGameMode>(GetWorld()->GetAuthGameMode())->PlayerData;
	PlayerData->OnTimeZeroGameOver.AddDynamic(this, &AWFCVisualizer::OnTimeZeroGameover);
}

// Called every frame
void AWFCVisualizer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsVisualizing)
	{
		ProcessSpawnTasks();
	}
}

void AWFCVisualizer::StartVisualization(int ActorPerFrame, const FWFCVisualizationData& Visualizationdata)
{
	this->ActorsPerFrame = ActorPerFrame;
	this->VisualizationData = Visualizationdata;
	TotalTiles = Visualizationdata.Tiles.Num();
	SetActorLocation(Visualizationdata.ParentLocation);
	SetActorRotation(Visualizationdata.ParentRotation);
	
	CurrentTileIndex = 0;
	bIsVisualizing = true;
}

void AWFCVisualizer::StopVisualization()
{
	ClearVisualization();
}

void AWFCVisualizer::ClearVisualization()
{
	for (AActor* Actor : SpawnedActors)
	{
		Actor->Destroy();
	}
	SpawnedActors.Empty();
}

void AWFCVisualizer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopVisualization();
	ClearVisualization();	
	Super::EndPlay(EndPlayReason);
}

void AWFCVisualizer::Destroyed()
{
	Super::Destroyed();
	
}

void AWFCVisualizer::OnTimeZeroGameover(UClass* ClassType, EGameOverType eType)
{
	ClearVisualization();
}

void AWFCVisualizer::ProcessSpawnTasks()
{
	if (SpawnedActors.Num() >= TotalTiles)
	{
		if (bIsVisualizing)
		{
			OnVisualizationFinished();
		}
		return;
	}
	int ActorSpawnedThisFrame = 0;
	while (ActorSpawnedThisFrame < ActorsPerFrame && CurrentTileIndex < TotalTiles)
	{
		FWFCVisualizationTile& CurrentTile = VisualizationData.Tiles[CurrentTileIndex];
		//如果tile类型为empty，则跳过
		if (CurrentTile.Category != EWFCTileCategory::Empty)
		{
			SpawnTileActor(CurrentTile);
		}
		else if (VisualizationData.bShowEmptyTiles)
		{
			SpawnTileActor(CurrentTile);
		}
		CurrentTileIndex++;
		ActorSpawnedThisFrame++;
		if (OnVisualizationProgress.IsBound())
		{
			OnVisualizationProgress.Broadcast(this, CurrentTileIndex, TotalTiles);
		}
		
		if (CurrentTileIndex % 10 == 0) // 每10个Actor显示一次进度
		{
			if (GEngine)
			{
				FString ProgressText = FString::Printf(TEXT("WFC Progress: %d/%d (%.1f%%)"), 
					CurrentTileIndex, TotalTiles, GetProgress() * 100.0f);
				GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Yellow, ProgressText);
			}
		}
	}
}

AActor* AWFCVisualizer::SpawnTileActor(const FWFCVisualizationTile& Tile)
{
	UWorld* World = GetWorld();
	AActor* TileActor = World->SpawnActor<AActor>();
	
	if (!TileActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("WFCVisualizer: Failed to spawn actor for tile"));
		return nullptr;
	}

	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(TileActor);
	MeshComponent->RegisterComponent();
	TileActor->SetRootComponent(MeshComponent);
	MeshComponent->SetStaticMesh(Tile.StaticMesh);

	if (Tile.Material)
	{
		MeshComponent->SetMaterial(0, Tile.Material);
	}

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	MeshComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	TileActor->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
	TileActor->SetActorRelativeLocation(Tile.Location);
	TileActor->SetActorRelativeRotation(Tile.Rotation);
	TileActor->SetActorTickEnabled(false);
	SpawnedActors.Add(TileActor);
	return TileActor;
}

void AWFCVisualizer::CreateSpawnTasks(const FWFCGenerationResult& GenerationResult)
{
}

void AWFCVisualizer::OnVisualizationFinished()
{
	bIsVisualizing = false;
	
	if (bShowProgress)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
				FString::Printf(TEXT("WFC Visualization Complete! Spawned %d actors"), SpawnedActors.Num()));
		}
	}
    
	// 触发完成事件
	if (OnVisualizationComplete.IsBound())
	{
		OnVisualizationComplete.Broadcast(this);
	}
}

