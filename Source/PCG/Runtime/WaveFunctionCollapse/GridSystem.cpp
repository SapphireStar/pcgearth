// Fill out your copyright notice in the Description page of Project Settings.


#include "GridSystem.h"


AGridSystem::AGridSystem()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AGridSystem::BeginPlay()
{   
    Super::BeginPlay();
}

void AGridSystem::SetGridCell(int32 X, int32 Y, int32 Z, AWFCBlock* NewCell)
{
    if (!IsValidIndex(X, Y, Z))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid grid index: (%d, %d, %d)"), X, Y, Z);
        return;
    }
    if (GridCells[X][Y][Z] && GridCells[X][Y][Z] == NewCell)
    {
        return;
    }
    NewCell->SetActorLocation(Grid[X][Y][Z]);
    NewCell->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
    if (GridCells[X][Y][Z]) GridCells[X][Y][Z]->Destroy();
    
    GridCells[X][Y][Z] = NewCell;
}

AWFCBlock* AGridSystem::GetGridCell(int32 X, int32 Y, int32 Z)
{
    if (!IsValidIndex(X, Y, Z))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid grid index: (%d, %d, %d)"), X, Y, Z);
        return nullptr;
    }

    return GridCells[X][Y][Z];
}

void AGridSystem::InitializeGrid(int x, int y, int z)
{
    ClearGrid();
    GridSizeX = x;
    GridSizeY = y;
    GridSizeZ = z;
    Grid.SetNum(GridSizeX);
    GridCells.SetNum(GridSizeX);
    
    for (int32 X = 0; X < GridSizeX; X++)
    {
        Grid[X].SetNum(GridSizeY);
        GridCells[X].SetNum(GridSizeY);
        
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            Grid[X][Y].SetNum(GridSizeZ);
            GridCells[X][Y].SetNum(GridSizeZ);
            
            for (int32 Z = 0; Z < GridSizeZ; Z++)
            {
                FVector WorldPos = CalculateWorldPosition(X, Y, Z);
                Grid[X][Y][Z] = WorldPos;
            }
        }
    }
}

bool AGridSystem::IsValidIndex(int32 X, int32 Y, int32 Z)
{
    return (X >= 0 && X < GridSizeX && Y >= 0 && Y < GridSizeY && Z >= 0 && Z < GridSizeZ);
}

FVector AGridSystem::CalculateWorldPosition(int32 X, int32 Y, int32 Z)
{
    FVector BasePosition = GetActorLocation();
    FVector GridOffset = FVector(X * CellSize, Y * CellSize, Z * CellSize);
    return BasePosition + GridOffset;
}

UStaticMeshComponent* AGridSystem::CreateMeshComponent(const FString& ComponentName)
{
    UStaticMeshComponent* MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(*ComponentName);
    
    if (MeshComponent)
    {
        MeshComponent->SetupAttachment(RootComponent);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    }
    
    return MeshComponent;
}

AWFCBlock* AGridSystem::CreateWFCBlock(int X, int Y, int Z, FVector Pos)
{
    TObjectPtr<AWFCBlock> cell;
    if (GetWorld())
    {
        cell = GetWorld()->SpawnActor<AWFCBlock>(AWFCBlock::StaticClass(), Pos, FRotator::ZeroRotator);
        GridCells[X][Y][Z] = cell;
    }
    else if (GEngine)
    {
        cell =GEngine->GetWorld()->SpawnActor<AWFCBlock>(AWFCBlock::StaticClass(), Pos, FRotator::ZeroRotator);
        GridCells[X][Y][Z] = cell;
    }
    if (cell)
    {
        cell->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn cell block"));
        return nullptr;
    }
    cell->GetComponentByClass<UStaticMeshComponent>()->SetStaticMesh(DefaultMesh);
    return cell;
}

void AGridSystem::ClearGrid()
{
    if (GridCells.Num()==0) return;
    for (int32 X = 0; X < GridCells.Num(); X++)
    {
        for (int32 Y = 0; Y < GridCells[X].Num(); Y++)
        {
            for (int32 Z = 0; Z < GridCells[X][Y].Num(); Z++)
            {
                if (GridCells[X][Y][Z])
                {
                    GridCells[X][Y][Z]->Destroy();
                }
            }
        }
    }
    
    Grid.Empty();
    GridCells.Empty();
}
