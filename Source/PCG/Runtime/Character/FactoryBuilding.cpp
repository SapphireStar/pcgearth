// Fill out your copyright notice in the Description page of Project Settings.


#include "FactoryBuilding.h"

#include "ItemAbilityComponent.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
AMiningBuilding::AMiningBuilding()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SphereCollision = CreateDefaultSubobject<USphereComponent>(FName("SphereCollision"));
	RootComponent = SphereCollision;
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	SphereCollision->SetCollisionObjectType(ECC_WorldStatic);
	SphereStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("SphereStaticMeshComponent"));
	SphereStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereStaticMeshComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AMiningBuilding::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMiningBuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	OnTickFactory(DeltaTime);
}

// 初始化工厂数据，使用volume来计算工厂的效率
void AMiningBuilding::BuildFactoryAt(FVector Position, int Volume, AMineSphere* MineSphere, float Radius, FOnAbilityActivated& OnActivated, FOnAbilityDeactivated& OnDeactivated)
{
	SetActorLocation(Position);
	PCGGameMode = Cast<APCGGameMode>(GetWorld()->GetAuthGameMode());
	if (!PCGGameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("FactoryBuilding: Gamemode is not PCGGameMode"));
		return;
	}
	PlayerData = PCGGameMode->PlayerData;
	this->MineSphere = MineSphere;
	FactoryInfo = PlayerData->GetPlayerData().FactoryInfo;
	
	OnBuildFactory(Volume);
	SphereCollision->SetSphereRadius(FactoryInfo.FactoryRadius);
	SphereStaticMeshComponent->SetStaticMesh(FactoryInfo.SphereStaticMesh);
	SphereStaticMeshComponent->SetMaterial(0, FactoryInfo.SphereMaterial);
	float MeshRadius = SphereStaticMeshComponent->GetStaticMesh()->GetBoundingBox().GetExtent().X;
	float MeshScale = FactoryInfo.FactoryRadius / MeshRadius;
	SphereStaticMeshComponent->SetWorldScale3D(FVector(MeshScale, MeshScale, MeshScale));

	OnActivated.AddDynamic(this, &AMiningBuilding::OnTerrainBuildAbilityActivated);
	OnDeactivated.AddDynamic(this, &AMiningBuilding::OnTerrainBuildAbilityDeactivated);
}

void AMiningBuilding::ActivateFactory()
{
	bIsFactoryActivated = true;
}

void AMiningBuilding::DeactivateFactory()
{
	bIsFactoryActivated = false;
}

void AMiningBuilding::OnBuildFactory(int Volume)
{
	FactoryEfficiency = Volume / FactoryInfo.EfficiencyDivider;
	
	bIsFactoryActivated = true;
}

void AMiningBuilding::OnTickFactory(float Deltatime)
{
	if (!bIsFactoryActivated) return;
	CurrentInterval +=  Deltatime;
	if (CurrentInterval >= FactoryInfo.MiningCD)
	{
		StartOneMining();
		CurrentInterval = 0;
	}
}

void AMiningBuilding::OnDestroyFactory()
{
	bIsFactoryActivated = false;
}

bool AMiningBuilding::StartOneMining()
{
	if (MineSphere->GetCanMine())
	{
		std::function<void(UPlayerDataComponent*)> pfun;
		int NewPlayerMine = PlayerData->GetPlayerStoneValue() + MineSphere->TryStartOneMine(FactoryEfficiency, pfun);
		pfun(PlayerData);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("FactoryBuilding: MineSphere is run out of Mine"));
	}
	return false;
}

void AMiningBuilding::OnTerrainBuildAbilityActivated(EAbilityType eType)
{
	SphereStaticMeshComponent->SetVisibility(true);
}

void AMiningBuilding::OnTerrainBuildAbilityDeactivated(EAbilityType eType)
{
	SphereStaticMeshComponent->SetVisibility(false);
}

