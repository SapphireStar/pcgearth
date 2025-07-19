// Fill out your copyright notice in the Description page of Project Settings.


#include "MineSphere.h"

#include "Components/SphereComponent.h"


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
	
}

// Called every frame
void AMineSphere::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMineSphere::InitializeMineSphere(AGeometryPlanet* planet)
{
	this->MotherWorldPlanet =  planet;
	
}

