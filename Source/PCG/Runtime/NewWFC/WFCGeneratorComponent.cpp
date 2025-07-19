// WFCGeneratorComponent.cpp - 修正后的实现文件
#include "WFCGeneratorComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Async/Async.h"

UWFCGeneratorComponent::UWFCGeneratorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // 设置默认配置
    Configuration.GridSize = FIntVector(5, 5, 3);
    Configuration.GenerationMode = EWFCGenerationMode::GroundFirst;
    Configuration.MaxIterations = 1000;
    Configuration.bEnableBacktracking = true;
    Configuration.BacktrackingDepth = 10;
}

void UWFCGeneratorComponent::BeginPlay()
{
    Super::BeginPlay();

    // 创建根可视化组件
    if (bVisualizeTiles && GetOwner())
    {
        RootVisualization = NewObject<USceneComponent>(GetOwner());
        RootVisualization->SetupAttachment(GetOwner()->GetRootComponent());
        RootVisualization->RegisterComponent();
    }

    // 初始化WFC核心
    WFCCore = MakeUnique<FWFCCore>();

    if (bAutoGenerateOnBeginPlay && TileSet)
    {
        StartGeneration();
    }
}

void UWFCGeneratorComponent::StartGeneration()
{
    StartGenerationWithCustomConfig(Configuration);
}

void UWFCGeneratorComponent::StartGenerationWithCustomConfig(const FWFCConfiguration& CustomConfig)
{
    if (bIsGenerating)
    {
        UE_LOG(LogTemp, Warning, TEXT("WFCGenerator: Generation already in progress"));
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
    CompleteTileSet->GenerateRotationVariants();

    UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Starting generation with grid size %s"), 
        *CustomConfig.GridSize.ToString());

    bIsGenerating = true;
    Configuration = CustomConfig;

    // 清除之前的结果
    ClearVisualization();
    
    // 初始化WFC核心
    if (!WFCCore->Initialize(CompleteTileSet, Configuration))
    {
        UE_LOG(LogTemp, Error, TEXT("WFCGenerator: Failed to initialize WFC core"));
        bIsGenerating = false;
        return;
    }

    // 根据配置选择同步或异步执行
    if (bUseAsyncGeneration)
    {
        ExecuteGenerationAsync();
    }
    else
    {
        ExecuteGeneration();
    }
}

void UWFCGeneratorComponent::StopGeneration()
{
    if (!bIsGenerating)
    {
        return;
    }

    bIsGenerating = false;
    UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Generation stopped"));

    // 如果有异步任务在运行，等待其完成
    if (GenerationFuture.IsValid())
    {
        // 注意：这里不应该在游戏线程上等待，实际项目中需要更优雅的处理
        GenerationFuture.Wait();
    }
}

void UWFCGeneratorComponent::ClearGeneration()
{
    StopGeneration();
    ClearVisualization();
    
    if (WFCCore)
    {
        WFCCore->Reset();
    }
    
    LastResult = FWFCGenerationResult();
    UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Generation cleared"));
}

void UWFCGeneratorComponent::SetTileSet(UWFCTileSet* NewTileSet)
{
    if (bIsGenerating)
    {
        UE_LOG(LogTemp, Warning, TEXT("WFCGenerator: Cannot change TileSet during generation"));
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

void UWFCGeneratorComponent::ExecuteGeneration()
{
    if (!WFCCore)
    {
        UE_LOG(LogTemp, Error, TEXT("WFCGenerator: WFC Core not initialized"));
        bIsGenerating = false;
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Executing synchronous generation"));

    FWFCGenerationResult Result = WFCCore->Generate();
    OnGenerationFinished(Result);
}

void UWFCGeneratorComponent::ExecuteGenerationAsync()
{
    UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Executing asynchronous generation"));

    // 在后台线程执行WFC生成
    GenerationFuture = Async(EAsyncExecution::ThreadPool, [this]() -> FWFCGenerationResult
    {
        if (WFCCore)
        {
            return WFCCore->Generate();
        }
        return FWFCGenerationResult();
    });

    // 在游戏线程上等待结果
    AsyncTask(ENamedThreads::GameThread, [this]()
    {
        if (GenerationFuture.IsValid())
        {
            FWFCGenerationResult Result = GenerationFuture.Get();
            OnGenerationFinished(Result);
        }
    });
}

void UWFCGeneratorComponent::OnGenerationFinished(const FWFCGenerationResult& Result)
{
    bIsGenerating = false;
    LastResult = Result;

    UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Generation finished - Success: %s, Iterations: %d, Time: %.3fs"), 
        Result.bSuccess ? TEXT("True") : TEXT("False"), 
        Result.IterationsUsed, 
        Result.GenerationTimeSeconds);

    if (!Result.bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("WFCGenerator: Generation failed: %s"), *Result.ErrorMessage);
    }

    // 创建可视化
    if (bVisualizeTiles && Result.bSuccess)
    {
        CreateVisualization(Result);
    }

    // 广播完成事件
    OnGenerationComplete.Broadcast(Result);
}

void UWFCGeneratorComponent::CreateVisualization(const FWFCGenerationResult& Result)
{
    if (!GetOwner() || !CompleteTileSet)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Creating visualization for %d tiles"), 
        Result.TileAssignments.Num());

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

    UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Created %d tile actors"), CreatedCount);
}

void UWFCGeneratorComponent::ClearVisualization()
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("WFCGenerator: Clearing %d spawned actors"), SpawnedActors.Num());

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
    if (!GetOwner() || !CompleteTileSet)
    {
        return nullptr;
    }

    FWFCTileDefinition TileDef = CompleteTileSet->GetTile(TileIndex);
    if (!TileDef.Mesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("WFCGenerator: Tile %d has no mesh"), TileIndex);
        return nullptr;
    }

    // 计算世界位置
    FVector WorldPosition = CoordinateToWorldPosition(Position);
    FRotator WorldRotation = TileDef.BaseRotation;

    // 创建Actor
    UWorld* World = GetOwner()->GetWorld();
    AActor* TileActor = World->SpawnActor<AActor>(AActor::StaticClass(), WorldPosition, WorldRotation);
    
    if (!TileActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("WFCGenerator: Failed to spawn actor for tile %d"), TileIndex);
        return nullptr;
    }

    // 设置名称
    TileActor->SetActorLabel(FString::Printf(TEXT("WFC_Tile_%s_%s"), 
        *Position.ToString(), *TileDef.TileName));

    // 创建静态网格组件
    UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(TileActor);
    MeshComponent->SetStaticMesh(TileDef.Mesh);
    
    if (TileDef.Material)
    {
        MeshComponent->SetMaterial(0, TileDef.Material);
    }

    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    
    TileActor->SetRootComponent(MeshComponent);
    MeshComponent->RegisterComponent();

    // 附加到根可视化组件
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
    
    // 简单计算，不做居中偏移
    FVector CellPosition = FVector(
        Coord.X * CellSize,
        Coord.Y * CellSize,
        Coord.Z * CellSize
    );

    return BasePosition + CellPosition;
}