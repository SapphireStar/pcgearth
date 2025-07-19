// Fill out your copyright notice in the Description page of Project Settings.


#include "WFCGenerator.h"
#include "WFCPropagatorDataGenerator.h"


// Sets default values
AWFCGenerator::AWFCGenerator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AWFCGenerator::BeginPlay()
{
	Super::BeginPlay();
	PropagatorDataGenerator = MakeShared<UWFCPropagatorDataGenerator>(SocketData, BaseTileData);
	PropagatorDataGenerator->GeneratePropagatorRules(Propagator);
	CompleteTileData = PropagatorDataGenerator->CompleteTileData;
	Weights.SetNum(CompleteTileData->Tiles.Num());
	for (int i = 0; i < CompleteTileData->Tiles.Num(); i++)
	{
		Weights[i] = CompleteTileData->Tiles[i].Weight;
	}
	
	//StartWFC(GetActorLocation(), GetActorRotation());
}

// Called every frame
void AWFCGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWFCGenerator::StartWFC(int SizeX, int SizeY, int SizeZ, const FVector& Position, const FRotator& Rotation, int Seed)
{
	WFCSolver = MakeShared<AWFCSolver>(SizeX, SizeY, SizeZ, Propagator[0].Num(), false, Propagator, Weights);
	auto grid = CreateWFCGrid(SizeZ, SizeY, SizeX, Position, Rotation);
	TArray<int> Result;
	WFCSolver->Run(static_cast<int>(RandomStream.FRandRange(0,5000.f)), SizeX * SizeY * SizeZ, Result);
	for (int i = 0; i < Result.Num(); i++)
	{
		int Z = i % SizeZ;
		int Y = (i / SizeZ) % SizeY;
		int X = (i / SizeZ) / SizeY;
		grid->SetGridCell(SizeZ - Z, Y, X, CreateBlock(Result[i]));
	}
	grid->SetActorRotation(Rotation);
	UE_LOG(LogTemp, Warning, TEXT("grid up: %f, %f, %f"), grid->GetActorUpVector().X, grid->GetActorUpVector().Y, grid->GetActorUpVector().Z);
}

AGridSystem* AWFCGenerator::CreateWFCGrid(int SizeX, int SizeY, int SizeZ, const FVector& Position,
	const FRotator& Rotation)
{
	auto grid = GetWorld()->SpawnActor<AGridSystem>();
	grid->SetActorLocation(Position);
	GridGenerator.Add(grid);
	grid->InitializeGrid(SizeX, SizeY, SizeZ);
	return grid;
}

AWFCBlock* AWFCGenerator::CreateBlock(int t)
{
	AWFCBlock* Block = GetWorld()->SpawnActor<AWFCBlock>();
	Block->Initialize(CompleteTileData->Tiles[t]);
	
	return Block;
}
