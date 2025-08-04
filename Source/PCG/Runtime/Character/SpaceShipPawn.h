// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "ItemAbilityComponent.h"
#include "Data/PlayerDataComponent.h"
#include "GameFramework/DefaultPawn.h"
#include "PCG/Runtime/WaveFunctionCollapse/WFCGenerator.h"
#include "SpaceShipPawn.generated.h"

class UGetResourceAbility;
class UTerrainDigAbility;
class UTerrainBuildCrafterAbility;
class UTerrainBuildAbility;
class AGridSelectionManager;
class UItemPlaceComponent;
class UDynamicMeshComponent;
struct FInputActionValue;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class UCapsuleComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityChanged, EAbilityType, OldAbility, EAbilityType, NewAbility);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerAimingResourceChanged, EFactoryResource, ResourceType);

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
	void Accelerate(const FInputActionValue& Value);
	void StopAccelerate(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Roll(const FInputActionValue& Value);
	void Rise(const FInputActionValue& Value);
	void Decline(const FInputActionValue& Value);
	void ProcessInput(float Deltatime);
	void CycleAbility(const FInputActionValue& Value);
	void SwitchAbility(const FInputActionValue& Value);
	void StartUseAbility(const FInputActionValue& Value);
	void KeepUsingAbility(const FInputActionValue& Value);
	void CompleteUseAbility(const FInputActionValue& Value);
	void CancelUseAbility(const FInputActionValue& Value);
	void CycleRecipe(const FInputActionValue& Value);

	int FindVertex(const FVector& Target, UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID);
	int FindLowestVertex(UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID);
	TObjectPtr<UItemAbilityComponent> CreateAbilityComponent(EAbilityType eAbilityType, FName AbilityName);

	void DrawDebugInfo();
	void DrawVectorDebugArrows(UStaticMeshComponent* MeshComponent, const FVector& Acceleration);

	float GetLaserRange() const { return LaserRange; }

	UFUNCTION(BlueprintCallable)
	int GetCurrentAbilityIndex() { return CurrentAbilityIndex; }

	UFUNCTION(BlueprintCallable)
	void ChangeCraftRecipe(FFactoryRecipeInfo RecipeInfo);

	UPROPERTY(BlueprintAssignable)
	FOnAbilityChanged OnAbilityChanged;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnPlayerAimingResourceChanged OnPlayerAimingResourceChanged;

	//FOV控制
	void UpdateFOVBasedOnSpeed(float DeltaTime);
	float CalculateTargetFOV(float CurrentSpeed);

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPlayerDataComponent> PlayerData;

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

	//Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> AccelerateAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> RollAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> UseAbilityAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> RiseAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> DeclineAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> DigTerrainAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> CycleAbilityAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> SwitchAbilityAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> CycleRecipeAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> CancelUseAbilityAction;

	//Control
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LowSpeedThreshold = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MediumSpeedThreshold = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighSpeedThreshold = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 5000.0f;

	UPROPERTY(BlueprintReadOnly)
	float BaseSpeed = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StopThreshold = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighResponsivenessFactor = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EmergencyBrakeFactor = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FastBrakeDamping = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MediumBrakeDamping = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SmoothBrakeDamping = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float YawSensitive = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchSensitive = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RollSensitive = 20.f;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip = "在飞船开始加速时，施加给飞船的初始的力"))
	float AccelerateImpulse = 100.f;

	//FOV控制
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultFOV = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedFOV = 110.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FOVInterpSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpeedForFOV = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeedForFOV = 3000.0f;
	
	    float CurrentFOV;
	
        float TargetFOV;

	//Movement
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

	TArray<TObjectPtr<UItemAbilityComponent>> Abilities;

	TObjectPtr<UTerrainBuildAbility> TerrainBuildAbility;

	TObjectPtr<UTerrainBuildCrafterAbility>  TerrainBuildCrafterAbility;

	TObjectPtr<UTerrainDigAbility>  TerrainDigAbility;

	TObjectPtr<UGetResourceAbility>   GetResourceAbility;

private:

	int CurrentAbilityIndex = 0;

	bool bIsBoosting = false;
	//int CurrentRecipeIndex = 0;
};
