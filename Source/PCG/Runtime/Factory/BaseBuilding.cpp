// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseBuilding.h"

#include "Components/SphereComponent.h"
#include "PCG/Runtime/PCGGameMode.h"
#include "PCG/Runtime/Character/SpaceShipPawn.h"


// Sets default values
ABaseBuilding::ABaseBuilding()
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

	SphereCollisionForTrace = CreateDefaultSubobject<USphereComponent>(FName("SphereCollisionForTrace"));
	SphereCollisionForTrace->SetupAttachment(RootComponent);
	SphereCollisionForTrace->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollisionForTrace->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Block);
	SphereCollisionForTrace->SetSphereRadius(500);
}

// Called when the game starts or when spawned
void ABaseBuilding::BeginPlay()
{
	Super::BeginPlay();
	Cast<ASpaceShipPawn>(GetWorld()->GetFirstPlayerController()->GetPawn())->OnAbilityChanged.AddDynamic(this, &ABaseBuilding::OnPlayerSwitchAbility);
}

// Called every frame
void ABaseBuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	OnTickFactory(DeltaTime);
}

// 初始化工厂数据，使用volume来计算工厂的效率
void ABaseBuilding::BuildFactoryAt(FVector Position, int volume, FFactoryInfo Info)
{
	SetActorLocation(Position);
	PCGGameMode = Cast<APCGGameMode>(GetWorld()->GetAuthGameMode());
	if (!PCGGameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("FactoryBuilding: Gamemode is not PCGGameMode"));
		return;
	}
	PlayerData = PCGGameMode->PlayerData;
	FactoryInfo = Info;
	
	OnBuildFactory(volume);

	//如果启用了工厂的碰撞检测，才对其碰撞体积初始化
	if (FactoryInfo.bEnableFactorySphereCollision)
	{
		SphereCollision->SetSphereRadius(FactoryInfo.FactoryRadius);
		SphereStaticMeshComponent->SetStaticMesh(FactoryInfo.SphereStaticMesh);
		SphereStaticMeshComponent->SetMaterial(0, FactoryInfo.SphereMaterial);
		float MeshRadius = SphereStaticMeshComponent->GetStaticMesh()->GetBoundingBox().GetExtent().X;
		float MeshScale = FactoryInfo.FactoryRadius / MeshRadius;
		SphereStaticMeshComponent->SetWorldScale3D(FVector(MeshScale, MeshScale, MeshScale));
	}
	
}

void ABaseBuilding::ActivateFactory()
{
	bIsFactoryActivated = true;
}

void ABaseBuilding::DeactivateFactory()
{
	bIsFactoryActivated = false;
}

FTooltipInfo ABaseBuilding::GetFactoryTooltipInfo_Implementation()
{
	return FTooltipInfo();
}

void ABaseBuilding::OnBuildFactory(int volume)
{
	this->Volume =  volume;
	FactoryEfficiency = volume / FactoryInfo.EfficiencyDivider;
}

void ABaseBuilding::OnTickFactory(float Deltatime)
{
	if (!bIsFactoryActivated) return;
	CurrentInterval +=  Deltatime;
	if (CurrentInterval >= FactoryInfo.ProduceCD)
	{
		StartOneProduce();
		CurrentInterval = 0;
	}
}

void ABaseBuilding::OnDestroyFactory()
{
	bIsFactoryActivated = false;
}

bool ABaseBuilding::StartOneProduce()
{
	return true;
}


void ABaseBuilding::OnPlayerSwitchAbility(EAbilityType OldAbility, EAbilityType NewAbility)
{
	if (NewAbility != EAbilityType::TerrainBuild && NewAbility != EAbilityType::TerrainBuildCrafter)
	{
		SphereStaticMeshComponent->SetVisibility(false);
	}
	else
	{
		SphereStaticMeshComponent->SetVisibility(true);
	}
}

void ABaseBuilding::OnTerrainBuildAbilityActivated(EAbilityType eType)
{
	SphereStaticMeshComponent->SetVisibility(true);
}

void ABaseBuilding::OnTerrainBuildAbilityDeactivated(EAbilityType eType)
{
	SphereStaticMeshComponent->SetVisibility(false);
}

