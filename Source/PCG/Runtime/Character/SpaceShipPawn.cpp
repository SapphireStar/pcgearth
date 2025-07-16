// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceShipPawn.h"

#include "EngineUtils.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "MaterialHLSLTree.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "PCG/Runtime/NewPlanet/GeometryPlanet.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "UDynamicMesh.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"

// Sets default values
ASpaceShipPawn::ASpaceShipPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CapsuleCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollision"));
	RootComponent = CapsuleCollision;

	MainBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainBody"));
	MainBody->SetupAttachment(CapsuleCollision);

	Front = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Front"));
	Front->SetupAttachment(MainBody);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CapsuleCollision);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

// Called when the game starts or when spawned
void ASpaceShipPawn::BeginPlay()
{
	Super::BeginPlay();
	if (GetWorld())
	{
		for (TActorIterator<AWFCGenerator> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (*ActorItr)
			{
				WFCGenerator = *ActorItr;
				WFCGenerator->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
			}
		}
	}
}

// Called every frame
void ASpaceShipPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	MainBody->SetWorldRotation(Camera->GetComponentRotation());
	//ProcessInput(DeltaTime);
}

// Called to bind functionality to input
void ASpaceShipPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASpaceShipPawn::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ASpaceShipPawn::StopMove);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASpaceShipPawn::Look);
		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &ASpaceShipPawn::Roll);
		EnhancedInputComponent->BindAction(SelectPointAction, ETriggerEvent::Completed, this,
		                                   &ASpaceShipPawn::SelectPoint);
		EnhancedInputComponent->BindAction(RiseAction, ETriggerEvent::Triggered, this, &ASpaceShipPawn::Rise);
	}
}

void ASpaceShipPawn::Move(const FInputActionValue& Value)
{
	int X = Value.Get<FVector2D>().X;
	int Y = Value.Get<FVector2D>().Y;
	FVector CameraForward = Camera->GetForwardVector() * Y;
	FVector CameraRight = Camera->GetRightVector() * X;
	FVector Move = CameraForward + CameraRight;
	CurrentAcceleration = Move;
	bIsAccelerating = true;
	AddActorWorldOffset(Move * GetWorld()->DeltaTimeSeconds * Speed);
}

void ASpaceShipPawn::StopMove(const FInputActionValue& Value)
{
	CurrentAcceleration = FVector::ZeroVector;
	bIsAccelerating = false;
}

void ASpaceShipPawn::Look(const FInputActionValue& Value)
{
	float X = Value.Get<FVector2D>().X;
	float Y = -Value.Get<FVector2D>().Y;

	FVector UpVector = Camera->GetUpVector();

	FQuat DeltaQuat = FQuat(UpVector, FMath::DegreesToRadians(X * YawSensitive) * GetWorld()->DeltaTimeSeconds);

	FQuat CurrentQuat = GetActorQuat();
	FQuat NewQuat = DeltaQuat * CurrentQuat;
	SetActorRotation(NewQuat);

	FVector RightVector = Camera->GetRightVector();

	DeltaQuat = FQuat(RightVector, FMath::DegreesToRadians(-Y * PitchSensitive) * GetWorld()->DeltaTimeSeconds);

	CurrentQuat = GetActorQuat();
	NewQuat = DeltaQuat * CurrentQuat;
	SetActorRotation(NewQuat);

	/*float X = Value.Get<FVector2D>().X;
	float Y = -Value.Get<FVector2D>().Y;
	FRotator yaw = FRotator(0, X * YawSensitive * GetWorld()->DeltaTimeSeconds, 0);
	FRotator pitch = FRotator(Y * PitchSensitive * GetWorld()->DeltaTimeSeconds, 0, 0);

	SpringArm->AddRelativeRotation(yaw+pitch);*/
}

void ASpaceShipPawn::Roll(const FInputActionValue& Value)
{
	float roll = Value.Get<float>();

	FVector ForwardVector = Camera->GetForwardVector();

	FQuat DeltaQuat = FQuat(ForwardVector,
	                        FMath::DegreesToRadians(roll * RollSensitive) * GetWorld()->DeltaTimeSeconds);

	FQuat CurrentQuat = GetActorQuat();
	FQuat NewQuat = DeltaQuat * CurrentQuat;
	SetActorRotation(NewQuat);
}

void ASpaceShipPawn::SelectPoint(const FInputActionValue& Value)
{
	FVector End = Front->GetComponentLocation() + Camera->GetForwardVector() * SelectRange;
	TArray<AActor*> ActorsToIgnore;
	FHitResult HitResult;
	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		Front->GetComponentLocation(),
		End,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		5.f);
	if (HitResult.bBlockingHit)
	{
		if (AGeometryPlanet* planet = Cast<AGeometryPlanet>(HitResult.GetActor()))
		{
			FGeometryScriptMeshSelection selection;
			UGeometryScriptLibrary_MeshSelectionFunctions::SelectMeshElementsInSphere(
				planet->GetDynamicMeshComponent()->GetDynamicMesh(),
				selection,
				HitResult.ImpactPoint,
				100.f,
				EGeometryScriptMeshSelectionType::Triangles,
				false,
				1
			);
			TArray<int32> indicesout;

			selection.ConvertToMeshIndexArray(
				planet->GetDynamicMeshComponent()->GetDynamicMesh()->GetMeshRef(),
				indicesout);
			if (indicesout.Num() > 0)
			{
				bool bIsValidVertex;
				auto mesh = planet->GetDynamicMeshComponent()->GetDynamicMesh();
				auto pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
					mesh, indicesout[0], bIsValidVertex);
				UE_LOG(LogTemp, Warning, TEXT("index: %f, %f, %f"), pos.X, pos.Y, pos.Z);
				UE_LOG(LogTemp, Warning, TEXT("impact pos: %f, %f, %f"), HitResult.ImpactPoint.X,
				       HitResult.ImpactPoint.Y, HitResult.ImpactPoint.Z);
				FVector normal = (pos - planet->GetActorLocation());
				normal.Normalize();
				planet->GetDynamicMeshComponent()->SetDynamicMesh(
					UGeometryScriptLibrary_MeshBasicEditFunctions::SetVertexPosition(
						mesh, indicesout[0], pos + normal * 100.f, bIsValidVertex));
				pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(mesh, indicesout[0], bIsValidVertex);
				UE_LOG(LogTemp, Warning, TEXT("index after change: %f, %f, %f"), pos.X, pos.Y, pos.Z);
				planet->GetDynamicMeshComponent()->NotifyMeshUpdated();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No selection found"));
			}
		}

		//临时用于让建筑与星球垂直
		FVector normal = HitResult.ImpactPoint - HitResult.GetActor()->GetActorLocation();
		normal.Normalize();
		FRotator rotation = UKismetMathLibrary::FindLookAtRotation(HitResult.GetActor()->GetActorLocation(), HitResult.GetActor()->GetActorLocation() + normal);

		GenerateBuilding(7, 7, 7, HitResult.ImpactPoint, rotation);
	}
}

void ASpaceShipPawn::Rise(const FInputActionValue& Value)
{
	AddActorLocalOffset(FVector::UpVector * RiseSpeed * GetWorld()->DeltaTimeSeconds);
}

void ASpaceShipPawn::ProcessInput(float Deltatime)
{
	auto prevtickgroup = PrimaryActorTick.TickGroup;
	PrimaryActorTick.TickGroup = TG_PostPhysics;

	
	ActorPrevLocation = ActorCurrentLocation;
	ActorCurrentLocation = GetActorLocation();
	CurrentVelocity = (ActorCurrentLocation - ActorPrevLocation) / Deltatime;
	float velLength = CurrentVelocity.Length();
	if (bIsAccelerating)
	{
		if (CurrentVelocity.Length() > 0)
		{
			FVector normal = FVector::CrossProduct(CurrentVelocity, CurrentAcceleration);
			FVector perpendicularVelocityComp = FVector::CrossProduct(CurrentAcceleration, normal);
			float length = CurrentVelocity.Dot(perpendicularVelocityComp);
			if (length < 10.f)
			{
				CurrentVelocity -= perpendicularVelocityComp;
			}
			else
			{
				CurrentVelocity -= perpendicularVelocityComp * Deceleration *Deltatime;
			}
		}
		CurrentVelocity += CurrentAcceleration * Acceleration * Deltatime;
	}
	else if (velLength > 1.f && CurrentVelocity.X == 0.f && CurrentVelocity.Y == 0.f && CurrentVelocity.Z == 0.f)
	{
		FVector velDir = CurrentVelocity.GetSafeNormal();
		CurrentVelocity -= velDir * Deceleration * Deltatime;
		if (CurrentVelocity.Length() < 10.f)
		{
			CurrentVelocity =  FVector::ZeroVector;
		}
	}
	else
	{
		CurrentVelocity = FVector::ZeroVector;
	}

	AddActorWorldOffset(CurrentVelocity * Deltatime);

	PrimaryActorTick.TickGroup = prevtickgroup;
	
}

void ASpaceShipPawn::GenerateBuilding(int SizeX, int SizeY, int SizeZ, const FVector& Location,
                                      const FRotator& Rotation)
{
	if (!WFCGenerator) return;
	WFCGenerator->StartWFC(SizeX, SizeY, SizeZ, Location, Rotation);
}
