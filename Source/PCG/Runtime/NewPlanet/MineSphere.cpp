// Fill out your copyright notice in the Description page of Project Settings.


#include "MineSphere.h"

#include "Components/SphereComponent.h"
#include "PCG/Runtime/Character/Data/PlayerDataComponent.h"


// Sets default values
AMineSphere::AMineSphere()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	RootComponent = Sphere;

	Sphere->SetSphereRadius(Radius);
}

// Called when the game starts or when spawned
void AMineSphere::BeginPlay()
{
	Super::BeginPlay();
	UpdateMineSphere(Radius);
}

// Called every frame
void AMineSphere::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	this->Radius = (float)RemainMinralCount/(float)TotalMineralCount * InitialRadius;
}

void AMineSphere::UpdateMineSphere(float radius)
{
	this->Radius = radius;
	InitialRadius = radius;
	Sphere->SetSphereRadius(radius);
	RemainMinralCount = (4/3 * PI * radius * radius * radius)/MineralDivider;
	TotalMineralCount =  RemainMinralCount;
}

void AMineSphere::SetMotherWorldPlanet(AActor* planet)
{
	this->MotherWorldPlanet =  planet;
	
}

int AMineSphere::TryStartOneMine(int Value, std::function<void(UPlayerDataComponent*)>& pfun)
{
	pfun = [](UPlayerDataComponent* PlayerData){};
	return 0;
}

