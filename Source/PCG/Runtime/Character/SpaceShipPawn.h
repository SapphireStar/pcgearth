// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "ItemAbilityComponent.h"
#include "GameFramework/DefaultPawn.h"
#include "PCG/Runtime/WaveFunctionCollapse/WFCGenerator.h"
#include "SpaceShipPawn.generated.h"

class AGridSelectionManager;
class UItemPlaceComponent;
class UDynamicMeshComponent;
struct FInputActionValue;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class UCapsuleComponent;
UCLASS()
class PCG_API ASpaceShipPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASpaceShipPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void SetupPlayerAbilityComponent();

	void Move(const FInputActionValue& Value);
	void StopMove(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Roll(const FInputActionValue& Value);
	void SelectPoint(const FInputActionValue& Value);
	void Rise(const FInputActionValue& Value);
	void DigTerrain(const FInputActionValue& Value);
	void ProcessInput(float Deltatime);
	void CycleAbility(const FInputActionValue& Value);
	void StartUseAbility(const FInputActionValue& Value);
	void KeepUsingAbility(const FInputActionValue& Value);
	void CompleteUseAbility(const FInputActionValue& Value);

	int FindVertex(const FVector& Target, UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID);
	int FindLowestVertex(UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID);
	TObjectPtr<UItemAbilityComponent> CreateAbilityComponent(EAbilityType eAbilityType);

	void DrawDebugInfo();
	void DrawVectorDebugArrows(UStaticMeshComponent* MeshComponent, const FVector& Acceleration);

	float GetLaserRange() const { return LaserRange; }
protected:
	//Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> MainBody;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> Front;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCapsuleComponent> CapsuleCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UItemPlaceComponent> ItemPlaceComponent;

	//Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> RollAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> UseAbilityAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> RiseAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> DigTerrainAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> CycleAbilityAction;

	//Control
	// 在你的 ASpaceShipPawn.h 文件中添加这些变量

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LowSpeedThreshold = 500.0f;  // 低速阈值

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MediumSpeedThreshold = 1500.0f;  // 中速阈值

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighSpeedThreshold = 3000.0f;  // 高速阈值

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 5000.0f;  // 最大速度限制

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StopThreshold = 10.0f;  // 停止阈值

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighResponsivenessFactor = 2.0f;  // 低速时的高响应系数

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EmergencyBrakeFactor = 3.0f;  // 紧急刹车系数

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FastBrakeDamping = 8.0f;  // 快速刹车阻尼

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MediumBrakeDamping = 4.0f;  // 中等刹车阻尼

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SmoothBrakeDamping = 2.0f;  // 平滑刹车阻尼
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float YawSensitive = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchSensitive = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RollSensitive = 20.f;

	float RollOffset = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RiseSpeed = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SelectRange = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaserRange = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Acceleration = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = 2.f, ClampMax = 10.f))
	float Deceleration = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RollAcceleration = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RollDeceleration = 200.f;

	FVector ActorPrevLocation;
	FVector ActorCurrentLocation;
	bool bIsAccelerating = false;
	FVector CurrentVelocity = FVector::ZeroVector;
	FVector CurrentAcceleration = FVector::ZeroVector;
	bool bIsRollAccelerating = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VertexSelectionTolerance = 500.f;

	//Ability
	bool bIsAbilityInitialized = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	EAbilityType CurrentAbilityType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UItemAbilityComponent> CurrentAbilityComponent;

	UPROPERTY()
	TArray<TObjectPtr<UItemAbilityComponent>> Abilities;

	UPROPERTY(BlueprintAssignable)
	FOnAbilityChanged OnAbilityChanged;

	int CurrentAbilityIndex = 0;
};
