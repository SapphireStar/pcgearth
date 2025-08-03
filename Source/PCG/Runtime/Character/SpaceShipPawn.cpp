#include "SpaceShipPawn.h"

#include "EngineUtils.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "GetResourceAbility.h"
#include "GridSelection.h"
#include "InputActionValue.h"
#include "ItemPlaceComponent.h"
#include "MaterialHLSLTree.h"
#include "TerrainBuildAbility.h"
#include "TerrainBuildCrafterAbility.h"
#include "TerrainDigAbility.h"
#include "TestWFCAbility.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
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

	SetupPlayerAbilityComponent();
}

// Called when the game starts or when spawned
void ASpaceShipPawn::BeginPlay()
{
	Super::BeginPlay();
	MainBody->SetSimulatePhysics(true);
	PlayerData = Cast<APCGGameMode>(GetWorld()->GetAuthGameMode())->PlayerData;

	BaseSpeed = MaxSpeed;
	Camera->SetFieldOfView(DefaultFOV);
	CurrentFOV = DefaultFOV;
}

// Called every frame
void ASpaceShipPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector vel = MainBody->ComponentVelocity.GetSafeNormal() * Speed;
	MainBody->ComponentVelocity.Set(vel.X, vel.Y, vel.Z);
	
	UpdateFOVBasedOnSpeed(DeltaTime);
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
		EnhancedInputComponent->BindAction(RiseAction, ETriggerEvent::Triggered, this, &ASpaceShipPawn::Rise);
		EnhancedInputComponent->BindAction(UseAbilityAction, ETriggerEvent::Started, this,
		                                   &ASpaceShipPawn::StartUseAbility);
		EnhancedInputComponent->BindAction(UseAbilityAction, ETriggerEvent::Triggered, this,
		                                   &ASpaceShipPawn::KeepUsingAbility);
		EnhancedInputComponent->BindAction(UseAbilityAction, ETriggerEvent::Completed, this,
		                                   &ASpaceShipPawn::CompleteUseAbility);
		EnhancedInputComponent->BindAction(CycleAbilityAction, ETriggerEvent::Triggered, this,
		                                   &ASpaceShipPawn::CycleAbility);
		EnhancedInputComponent->BindAction(SwitchAbilityAction, ETriggerEvent::Triggered, this,
		                                   &ASpaceShipPawn::SwitchAbility);
		EnhancedInputComponent->BindAction(CycleRecipeAction, ETriggerEvent::Completed, this,&ASpaceShipPawn::CycleRecipe);
		EnhancedInputComponent->BindAction(AccelerateAction, ETriggerEvent::Started, this, &ASpaceShipPawn::Accelerate);
		EnhancedInputComponent->BindAction(AccelerateAction,ETriggerEvent::Completed, this, &ASpaceShipPawn::StopAccelerate);
		EnhancedInputComponent->BindAction(DeclineAction, ETriggerEvent::Triggered, this, &ASpaceShipPawn::Decline);
		EnhancedInputComponent->BindAction(CancelUseAbilityAction, ETriggerEvent::Completed, this, &ASpaceShipPawn::CancelUseAbility);
	}
}

void ASpaceShipPawn::SetupPlayerAbilityComponent()
{
	TObjectPtr<UItemAbilityComponent> TerrainBuild = CreateAbilityComponent(
		EAbilityType::TerrainBuild, FName("TerrainBuild"));
	
	TObjectPtr<UTerrainBuildCrafterAbility> TerrainBuildCrafter = Cast<UTerrainBuildCrafterAbility>(
		CreateAbilityComponent(EAbilityType::TerrainBuildCrafter, FName("TerrainBuildCrafter")));
	
	TObjectPtr<UItemAbilityComponent> TerrainDig =
		CreateAbilityComponent(EAbilityType::TerrainDig, FName("TerrainDig"));
	
	TObjectPtr<UGetResourceAbility> GetResource = Cast<UGetResourceAbility>(CreateAbilityComponent(
		EAbilityType::GetResource, FName("GetResource")));
	GetResource->SetLaserRange(LaserRange);
	
	if (TerrainBuild != nullptr)
		Abilities.Add(TerrainBuild);
	if (TerrainDig != nullptr)
		Abilities.Add(TerrainDig);
	if (GetResource != nullptr)
		Abilities.Add(GetResource);
	if (TerrainBuildCrafter != nullptr)
		Abilities.Add(TerrainBuildCrafter);


	CurrentAbilityComponent = Abilities[CurrentAbilityIndex];
	CurrentAbilityComponent->OnActivateAbility();
	bIsAbilityInitialized = true;
}

void ASpaceShipPawn::Move(const FInputActionValue& Value)
{
	FVector2D InputVector = Value.Get<FVector2D>();
	float X = InputVector.X;
	float Y = InputVector.Y;

	FVector CameraForward = Camera->GetForwardVector() * Y;
	FVector CameraRight = Camera->GetRightVector() * X;
	FVector DesiredMoveDirection = (CameraForward + CameraRight).GetSafeNormal();

	FVector CurrentVelocity = MainBody->GetPhysicsLinearVelocity();
	float CurrentSpeed = CurrentVelocity.Size();
	FVector CurrentDirection = CurrentVelocity.GetSafeNormal();

	bool bHasInput = !DesiredMoveDirection.IsNearlyZero();

	if (bHasInput)
	{
		bIsAccelerating = true;
		CurrentAcceleration = DesiredMoveDirection;

		float DotProduct = FVector::DotProduct(CurrentDirection, DesiredMoveDirection);
		float Angle = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)) * 180.0f / PI;

		if (CurrentSpeed < LowSpeedThreshold)
		{
			MainBody->SetLinearDamping(0.1f);
			MainBody->AddForce(DesiredMoveDirection * Acceleration * HighResponsivenessFactor);
		}
		else
		{
			if (Angle < 90.f)
			{
				MainBody->SetLinearDamping(0.2f);
				MainBody->AddForce(DesiredMoveDirection * Acceleration);

				FVector AdjustedVelocity = CurrentSpeed * DesiredMoveDirection;
				MainBody->SetPhysicsLinearVelocity(AdjustedVelocity);
			}
			else
			{
				MainBody->SetLinearDamping(FastBrakeDamping);
				MainBody->AddForce(-CurrentDirection * Acceleration * EmergencyBrakeFactor);
				MainBody->AddForce(DesiredMoveDirection * Acceleration * 0.4f);
			}
		}

	}
	else
	{
		bIsAccelerating = false;
		CurrentAcceleration = FVector::ZeroVector;

		if (CurrentSpeed > StopThreshold)
		{
			if (CurrentSpeed > HighSpeedThreshold)
			{
				MainBody->SetLinearDamping(FastBrakeDamping);
				MainBody->AddForce(-CurrentDirection * Deceleration * EmergencyBrakeFactor);
			}
			else if (CurrentSpeed > MediumSpeedThreshold)
			{
				MainBody->SetLinearDamping(MediumBrakeDamping);
				MainBody->AddForce(-CurrentDirection * Deceleration * 2.0f);
			}
			else
			{
				MainBody->SetLinearDamping(SmoothBrakeDamping);
			}
		}
		else
		{
			MainBody->SetPhysicsLinearVelocity(FVector::ZeroVector);
			MainBody->SetLinearDamping(0.1f);
		}
	}

	if (CurrentSpeed > MaxSpeed)
	{
		FVector LimitedVelocity = CurrentDirection * MaxSpeed;
		MainBody->SetPhysicsLinearVelocity(LimitedVelocity);
	}
}

void ASpaceShipPawn::Accelerate(const FInputActionValue& Value)
{
	MainBody->AddImpulse(CurrentAcceleration * AccelerateImpulse, NAME_None, true);
	float SpeedMultiplier = PlayerData->GetPlayerAbilityPropertyValue(EPlayerAbilityPropertyType::EPAPT_SpeedMultiplier);
	MaxSpeed *= SpeedMultiplier;
	bIsBoosting = true;
}

void ASpaceShipPawn::StopAccelerate(const FInputActionValue& Value)
{
	MaxSpeed = BaseSpeed;
	bIsBoosting = false;
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

void ASpaceShipPawn::Rise(const FInputActionValue& Value)
{
	AddActorLocalOffset(FVector::UpVector * RiseSpeed * GetWorld()->DeltaTimeSeconds);
}

void ASpaceShipPawn::Decline(const FInputActionValue& Value)
{
	AddActorLocalOffset(FVector::DownVector * RiseSpeed * GetWorld()->DeltaTimeSeconds);
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

void ASpaceShipPawn::CycleAbility(const FInputActionValue& Value)
{
	UItemAbilityComponent* OldAbility = CurrentAbilityComponent;
	float Axis = Value.Get<float>();
	if (Axis < 0)
	{
		CurrentAbilityIndex = (CurrentAbilityIndex + 1) % Abilities.Num();
	}
	else if (Axis > 0)
	{
		CurrentAbilityIndex = (CurrentAbilityIndex - 1);
		if (CurrentAbilityIndex < 0)
		{
			CurrentAbilityIndex = Abilities.Num() - 1;
		}
	}
	CurrentAbilityComponent = Abilities[CurrentAbilityIndex];
	UItemAbilityComponent* NewAbility = CurrentAbilityComponent;

	OldAbility->OnDeactivateAbility();
	NewAbility->OnActivateAbility();
	OnAbilityChanged.Broadcast(OldAbility->AbilityType, NewAbility->AbilityType);

	if (GEngine)
	{
		FString name = UEnum::GetValueAsString(Abilities[CurrentAbilityIndex]->AbilityType);
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, FString::Printf(TEXT("Axis: %s"), *name));
	}
}

void ASpaceShipPawn::SwitchAbility(const FInputActionValue& Value)
{
	UItemAbilityComponent* OldAbility = CurrentAbilityComponent;
	UE_LOG(LogTemp, Warning, TEXT("Switch Ability: %f"), Value.Get<float>());
	int Index = FMath::RoundToInt(Value.Get<float>()) - 1;
	CurrentAbilityIndex = (Index) % Abilities.Num();

	CurrentAbilityComponent = Abilities[CurrentAbilityIndex];
	UItemAbilityComponent* NewAbility = CurrentAbilityComponent;

	OldAbility->OnDeactivateAbility();
	NewAbility->OnActivateAbility();
	OnAbilityChanged.Broadcast(OldAbility->AbilityType, NewAbility->AbilityType);

	if (GEngine)
	{
		FString name = UEnum::GetValueAsString(Abilities[CurrentAbilityIndex]->AbilityType);
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, FString::Printf(TEXT("Axis: %s"), *name));
	}
}

void ASpaceShipPawn::StartUseAbility(const FInputActionValue& Value)
{
	CurrentAbilityComponent->OnStartUseAbility(Front, Camera);
}

void ASpaceShipPawn::KeepUsingAbility(const FInputActionValue& Value)
{
	CurrentAbilityComponent->OnKeepUsingAbility(Front, Camera);
}

void ASpaceShipPawn::CompleteUseAbility(const FInputActionValue& Value)
{
	CurrentAbilityComponent->OnCompleteUseAbility(Front, Camera);
}

void ASpaceShipPawn::CancelUseAbility(const FInputActionValue& Value)
{
	CurrentAbilityComponent->OnCancelUseAbility();
}

void ASpaceShipPawn::CycleRecipe(const FInputActionValue& Value)
{
	if (CurrentAbilityComponent->AbilityType == EAbilityType::TerrainBuildCrafter)
	{
		int total = PlayerData->GetPlayerData().RecipeInfos.Num();
		PlayerData->ChangePlayerRecipeIndex(PlayerData->GetPlayerCurrentRecipeIndex() + 1);
		int index = (PlayerData->GetPlayerCurrentRecipeIndex()) % total;
		ChangeCraftRecipe(PlayerData->GetPlayerData().RecipeInfos[index]);
	}
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

TObjectPtr<UItemAbilityComponent> ASpaceShipPawn::CreateAbilityComponent(EAbilityType eAbilityType, FName AbilityName)
{
	UItemAbilityComponent* AbilityComponent = nullptr;
	switch (eAbilityType)
	{
	case EAbilityType::None:
		break;
	case EAbilityType::TerrainBuild:
		AbilityComponent = CreateDefaultSubobject<UTerrainBuildAbility>(AbilityName);
		AbilityComponent->AbilityType = EAbilityType::TerrainBuild;
		break;
	case EAbilityType::TerrainBuildCrafter:
		AbilityComponent = CreateDefaultSubobject<UTerrainBuildCrafterAbility>(AbilityName);
		AbilityComponent->AbilityType = EAbilityType::TerrainBuildCrafter;
		break;
	case EAbilityType::TerrainDig:
		AbilityComponent = CreateDefaultSubobject<UTerrainDigAbility>(AbilityName);
		AbilityComponent->AbilityType = EAbilityType::TerrainDig;
		break;
	case EAbilityType::GetResource:
		AbilityComponent = CreateDefaultSubobject<UGetResourceAbility>(AbilityName);
		AbilityComponent->AbilityType = EAbilityType::GetResource;
		break;
	case EAbilityType::TestWFC:
		AbilityComponent = CreateDefaultSubobject<UTestWFCAbility>(AbilityName);
		AbilityComponent->AbilityType = EAbilityType::TestWFC;
		break;
	default:
		UE_LOG(LogTemp, Error,
		       TEXT("SpaceShipPawn::CreateAbilityComponent: Haven't define the initialization for EAbilityType: %d"),
		       static_cast<int>(eAbilityType));
	}

	if (!AbilityComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpaceShipPawn: Created Component is nullptr"));
	}
	return AbilityComponent;
}

void ASpaceShipPawn::ChangeCraftRecipe(FFactoryRecipeInfo RecipeInfo)
{
	Cast<UTerrainBuildCrafterAbility>(Abilities[CurrentAbilityIndex])->SetFactoryRecipeInfo(RecipeInfo);
	PlayerData->ChangePlayerCurrentRecipe(RecipeInfo);
}

void ASpaceShipPawn::UpdateFOVBasedOnSpeed(float DeltaTime)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PC->PlayerCameraManager) return;

	// 获取当前速度（忽略Z轴，只考虑水平移动）
	FVector HorizontalVelocity = GetVelocity();
	HorizontalVelocity.Z = 0;
	float CurrentSpeed = HorizontalVelocity.Size();

	// 计算目标FOV
	float TargetFOV = CalculateTargetFOV(CurrentSpeed);

	// 平滑插值到目标FOV
	CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, FOVInterpSpeed);
	PC->PlayerCameraManager->SetFOV(CurrentFOV);
}

float ASpaceShipPawn::CalculateTargetFOV(float CurrentSpeed)
{
	if (CurrentSpeed <= MinSpeedForFOV)
	{
		return DefaultFOV;
	}
    
	if (CurrentSpeed >= MaxSpeedForFOV)
	{
		return SpeedFOV;
	}

	float SpeedRange = MaxSpeedForFOV - MinSpeedForFOV;
	float SpeedRatio = (CurrentSpeed - MinSpeedForFOV) / SpeedRange;
    

		SpeedRatio = FMath::SmoothStep(0.0f, 1.0f, SpeedRatio);
	
    
	return FMath::Lerp(DefaultFOV, SpeedFOV, SpeedRatio);
}

void ASpaceShipPawn::DrawDebugInfo()
{
}
