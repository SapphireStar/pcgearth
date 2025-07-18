// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceShipPawn.h"

#include "EngineUtils.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "GridSelection.h"
#include "InputActionValue.h"
#include "ItemPlaceComponent.h"
#include "MaterialHLSLTree.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "PCG/Runtime/NewPlanet/GeometryPlanet.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "UDynamicMesh.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"

void ASpaceShipPawn::DrawVectorDebugArrows(UStaticMeshComponent* MeshComponent, const FVector& Acceleration)
{
	if (!MeshComponent)
	{
		return;
	}

	// 获取Actor位置作为起点
	FVector ActorLocation = GetActorLocation();

	// 获取物理组件的速度
	FVector Velocity = MeshComponent->GetPhysicsLinearVelocity();

	// 配置箭头显示参数
	float ArrowSize = 50.0f; // 箭头大小
	float ArrowThickness = 5.0f; // 箭头粗细
	float LifeTime = 0.0f; // 0表示只显示一帧，-1表示持续显示
	uint8 DepthPriority = 0; // 深度优先级

	// 速度向量的缩放系数（可根据需要调整）
	float VelocityScale = 0.1f; // 速度通常较大，需要缩放
	float AccelerationScale = 100.0f; // 加速度通常较小，需要放大

	// 绘制速度箭头（绿色）
	if (!Velocity.IsNearlyZero())
	{
		FVector VelocityEnd = ActorLocation + (Velocity * VelocityScale);

		DrawDebugDirectionalArrow(
			GetWorld(),
			ActorLocation, // 起点
			VelocityEnd, // 终点
			ArrowSize, // 箭头大小
			FColor::Green, // 颜色：绿色代表速度
			false, // 是否持续显示
			LifeTime, // 生命周期
			DepthPriority, // 深度优先级
			ArrowThickness // 粗细
		);

		// 添加文本标签
		DrawDebugString(
			GetWorld(),
			VelocityEnd + FVector(0, 0, 20), // 稍微偏移显示文本
			FString::Printf(TEXT("Velocity: %.1f"), Velocity.Size()),
			nullptr,
			FColor::Green,
			LifeTime
		);
	}

	// 绘制加速度箭头（红色）
	if (!Acceleration.IsNearlyZero())
	{
		FVector AccelerationEnd = ActorLocation + (Acceleration * AccelerationScale);

		DrawDebugDirectionalArrow(
			GetWorld(),
			ActorLocation, // 起点
			AccelerationEnd, // 终点
			ArrowSize, // 箭头大小
			FColor::Red, // 颜色：红色代表加速度
			false, // 是否持续显示
			LifeTime, // 生命周期
			DepthPriority, // 深度优先级
			ArrowThickness // 粗细
		);

		// 添加文本标签
		DrawDebugString(
			GetWorld(),
			AccelerationEnd + FVector(0, 0, 40), // 稍微偏移显示文本
			FString::Printf(TEXT("Acceleration: %.1f"), Acceleration.Size()),
			nullptr,
			FColor::Red,
			LifeTime
		);
	}

	// 可选：绘制Actor位置的参考点
	DrawDebugSphere(
		GetWorld(),
		ActorLocation,
		20.0f, // 半径
		12, // 段数
		FColor::Blue, // 颜色
		false, // 是否持续显示
		LifeTime, // 生命周期
		DepthPriority, // 深度优先级
		2.0f // 线条粗细
	);
}

// Sets default values
ASpaceShipPawn::ASpaceShipPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	MainBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainBody"));
	RootComponent = MainBody;


	CapsuleCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollision"));
	CapsuleCollision->SetupAttachment(MainBody);

	Front = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Front"));
	Front->SetupAttachment(CapsuleCollision);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(MainBody);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	ItemPlaceComponent = CreateDefaultSubobject<UItemPlaceComponent>(TEXT("ItemPlaceComponent"));
}

// Called when the game starts or when spawned
void ASpaceShipPawn::BeginPlay()
{
	Super::BeginPlay();
	MainBody->SetSimulatePhysics(true);
	if (!WFCGenerator && GetWorld())
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
	//MainBody->SetWorldRotation(Camera->GetComponentRotation());
	//ProcessInput(DeltaTime);
	FVector vel = MainBody->ComponentVelocity.GetSafeNormal() * Speed;
	MainBody->ComponentVelocity.Set(vel.X, vel.Y, vel.Z);
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
		EnhancedInputComponent->BindAction(DigTerrainAction, ETriggerEvent::Completed, this, &ASpaceShipPawn::DigTerrain);
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
	MainBody->AddForce(Move * Acceleration);

	DrawVectorDebugArrows(MainBody, Move.GetSafeNormal());
	float angle = FMath::Acos(FVector::DotProduct(MainBody->GetPhysicsLinearVelocity().GetSafeNormal(),
	                                              Move.GetSafeNormal())) * 180.f / PI;
	if (angle > 15)
	{
		MainBody->SetLinearDamping(1.f);
	}
	else if (angle > 45)
	{
		MainBody->SetLinearDamping(2.f);
	}
	else if (angle > 80)
	{
		MainBody->SetLinearDamping(Deceleration);
	}
	else
	{
		MainBody->SetLinearDamping(0.01f);
	}
	//AddActorWorldOffset(Move * GetWorld()->DeltaTimeSeconds * Speed);
}

void ASpaceShipPawn::StopMove(const FInputActionValue& Value)
{
	CurrentAcceleration = FVector::ZeroVector;
	bIsAccelerating = false;
	MainBody->SetLinearDamping(Deceleration);
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
	ItemPlaceComponent->SelectPoint(Front, Camera);
	return;
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
			FVector ImpactRelativePoint = HitResult.ImpactPoint - planet->GetActorLocation();
			FGeometryScriptMeshSelection selection;
			UGeometryScriptLibrary_MeshSelectionFunctions::SelectMeshElementsInSphere(
				planet->GetDynamicMeshComponent()->GetDynamicMesh(),
				selection,
				ImpactRelativePoint,
				VertexSelectionTolerance,
				EGeometryScriptMeshSelectionType::Vertices,
				false,
				1
			);
			TArray<int32> indicesout;

			selection.ConvertToMeshIndexArray(
				planet->GetDynamicMeshComponent()->GetDynamicMesh()->GetMeshRef(),
				indicesout);


			/*bool bIsValidVertex;
			auto mesh = planet->GetDynamicMeshComponent()->GetDynamicMesh();
			int VertexID = FindVertex(HitResult.ImpactPoint, planet->GetDynamicMeshComponent());
			auto pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(mesh, VertexID, bIsValidVertex);

			FVector normal = (pos - planet->GetActorLocation());
			normal.Normalize();
			UGeometryScriptLibrary_MeshBasicEditFunctions::SetVertexPosition(
				mesh, VertexID, pos + normal * 100.f, bIsValidVertex);
			
			pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(mesh, VertexID, bIsValidVertex);
			UE_LOG(LogTemp, Warning, TEXT("impact point: %f, %f, %f"), HitResult.ImpactPoint.X, HitResult.ImpactPoint.Y, HitResult.ImpactPoint.Z);
			UE_LOG(LogTemp, Warning, TEXT("index after change: %f, %f, %f"), pos.X, pos.Y, pos.Z);
			planet->GetDynamicMeshComponent()->NotifyMeshUpdated();*/


			if (indicesout.Num() > 0)
			{
				int LowestVertexID = FindLowestVertex(planet->GetDynamicMeshComponent(), indicesout);
				bool bIsValidVertex;
				auto mesh = planet->GetDynamicMeshComponent()->GetDynamicMesh();

				auto lowesetPos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
					mesh, LowestVertexID, bIsValidVertex);
				float lowestLength = (lowesetPos).Length();
				for (int i : indicesout)
				{
					if (i != LowestVertexID)
					{
						auto pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
							mesh, i, bIsValidVertex);
						FVector normal = (pos);
						normal.Normalize();
						UGeometryScriptLibrary_MeshBasicEditFunctions::SetVertexPosition(
							mesh, i, normal * lowestLength, bIsValidVertex);
					}
				}

				planet->GetDynamicMeshComponent()->NotifyMeshUpdated();
				planet->GetDynamicMeshComponent()->UpdateCollision();

				//临时用于让建筑与星球垂直
				FVector normal = HitResult.ImpactPoint - HitResult.GetActor()->GetActorLocation();
				normal.Normalize();
				FRotator rotation = UKismetMathLibrary::FindLookAtRotation(HitResult.GetActor()->GetActorLocation(),
																		   HitResult.GetActor()->GetActorLocation() + normal);

				GenerateBuilding(7, 7, 7, (HitResult.ImpactPoint - planet->GetActorLocation()).GetSafeNormal() * lowestLength + planet->GetActorLocation(), rotation);

				/*int ClosestVertexID = FindVertex(HitResult.ImpactPoint, planet->GetDynamicMeshComponent(),indicesout);

				
				bool bIsValidVertex;
				auto mesh = planet->GetDynamicMeshComponent()->GetDynamicMesh();
				auto pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
					mesh, ClosestVertexID, bIsValidVertex);
				UE_LOG(LogTemp, Warning, TEXT("index: %f, %f, %f"), pos.X, pos.Y, pos.Z);
				UE_LOG(LogTemp, Warning, TEXT("impact pos: %f, %f, %f"), HitResult.ImpactPoint.X,
				       HitResult.ImpactPoint.Y, HitResult.ImpactPoint.Z);
				FVector normal = (pos - planet->GetActorLocation());
				normal.Normalize();

				UGeometryScriptLibrary_MeshBasicEditFunctions::SetVertexPosition(
					mesh, ClosestVertexID, pos + normal * 100.f, bIsValidVertex);
				pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(mesh, ClosestVertexID, bIsValidVertex);
				UE_LOG(LogTemp, Warning, TEXT("index after change: %f, %f, %f"), pos.X, pos.Y, pos.Z);
				planet->GetDynamicMeshComponent()->NotifyMeshUpdated();
				planet->GetDynamicMeshComponent()->UpdateCollision();*/
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No selection found"));
			}
		}


	}
}

void ASpaceShipPawn::Rise(const FInputActionValue& Value)
{
	AddActorLocalOffset(FVector::UpVector * RiseSpeed * GetWorld()->DeltaTimeSeconds);
}

void ASpaceShipPawn::DigTerrain(const FInputActionValue& Value)
{
	ItemPlaceComponent->DigTerrain(Front, Camera);
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
				CurrentVelocity -= perpendicularVelocityComp * Deceleration * Deltatime;
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
			CurrentVelocity = FVector::ZeroVector;
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

int ASpaceShipPawn::FindVertex(const FVector& target, UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID)
{
	auto DynamicMesh = DynamicMeshComp->GetDynamicMesh();
	bool bHasIdGroup;

	float min = FLT_MAX;
	int minID = 0;
	for (auto id : VertexID)
	{
		FVector pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(DynamicMesh, id, bHasIdGroup);
		if (FVector::Dist(pos, target) < min)
		{
			min = FVector::Dist(pos, target);
			minID = id;
		}
	}
	return minID;
}

int ASpaceShipPawn::FindLowestVertex(UDynamicMeshComponent* DynamicMeshComp,
                                     TArray<int32> VertexID)
{
	auto DynamicMesh = DynamicMeshComp->GetDynamicMesh();
	bool bHasIdGroup;
	FVector planetPos = DynamicMeshComp->GetOwner()->GetActorLocation();

	float min = FLT_MAX;
	int minID = 0;
	for (auto id : VertexID)
	{
		FVector pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(DynamicMesh, id, bHasIdGroup);
		float dist = FVector::Dist(pos, planetPos);
		if (dist < min)
		{
			min = dist;
			minID = id;
		}
	}
	return minID;
}
