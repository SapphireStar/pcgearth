// Fill out your copyright notice in the Description page of Project Settings.


#include "TileISMManager.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "PCG/Runtime/PCGGameMode.h"


// Sets default values
ATileISMManager::ATileISMManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATileISMManager::BeginPlay()
{
	Super::BeginPlay();
	InitializeIsm();
	APCGGameMode* gamemode = Cast<APCGGameMode>(GetWorld()->GetAuthGameMode());
	gamemode->PlayerData->OnTimeZeroGameOver.AddDynamic(this, &ATileISMManager::OnRestartGame);
}

// Called every frame
void ATileISMManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!TileToUpdate.IsEmpty())
	{
		for (const auto& it : TileToUpdate)
		{
			auto ISMComp = ISMComponents[it.Key];
			FTransform Transform;
			ISMComp->GetInstanceTransform(it.Value,Transform, true);
			float AnimValue = TileAnimProgress[it.Key ^ it.Value];
			if (AnimValue >= 1)
			{
				Transform.SetScale3D(FVector::OneVector);
				ISMComp->UpdateInstanceTransform(it.Value, Transform, true, false, true);
				TileToUpdate.RemoveNode(it);
			}
			else
			{
				float scale = CurveData.GetRichCurve()->Eval(AnimValue);
				TileAnimProgress[it.Key ^ it.Value] += DeltaTime * AnimSpeed;
				FVector NewScale = FVector::Max(FVector::One() * 0.01f, FVector::One() * scale);
				
				Transform.SetScale3D(NewScale);
				ISMComp->UpdateInstanceTransform(it.Value, Transform, true, false, true);
			}
		}
	}
}

void ATileISMManager::InitializeIsm()
{
	if (!TileSet)
	{
		UE_LOG(LogTemp, Error, TEXT("ATileISMManager::InitializeIs: Can't find TileSet"));
		return;
	}
	for (int i = 0; i < TileSet->Tiles.Num(); i++)
	{
		UStaticMesh* mesh = TileSet->Tiles[i].Mesh;
		UMaterial* material = TileSet->Tiles[i].Material;
		UInstancedStaticMeshComponent* NewISM = Cast<UInstancedStaticMeshComponent>(AddComponentByClass(UInstancedStaticMeshComponent::StaticClass(), false, FTransform::Identity, true));
		NewISM->RegisterComponent();
		NewISM->SetStaticMesh(mesh);
		NewISM->SetMaterial(0, material);
		ISMComponents.Add(NewISM);
	}

}

void ATileISMManager::SpawnTileAt(int tile, FVector location, FRotator rotation)
{
	if (tile >= ISMComponents.Num() || tile < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("ATileISMManager::SpawnTileAt: Invalid tile ID: %d"), tile);
		return;
	}
	FTransform transform = FTransform::Identity;
	transform.SetLocation(location);
	transform.SetRotation(rotation.Quaternion());
	transform.SetScale3D(FVector::One() * 0.01f);
	ISMComponents[tile]->AddInstance(transform, true);
	TileToUpdate.AddTail({tile, ISMComponents[tile]->GetNumInstances() - 1});
	TileAnimProgress.Add(tile ^ ISMComponents[tile]->GetNumInstances() - 1, 0.f);
}

void ATileISMManager::ClearAllTiles()
{
	for (int i = 0; i < ISMComponents.Num(); i++)
	{
		ISMComponents[i]->ClearInstances();
	}
	TileToUpdate.Empty();
	TileAnimProgress.Empty();
}

void ATileISMManager::OnRestartGame(UClass* DataClassType, EGameOverType eGameOverType)
{
	ClearAllTiles();
}

