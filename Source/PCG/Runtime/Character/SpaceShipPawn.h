// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
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

	void Move(const FInputActionValue& Value);
	void StopMove(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Roll(const FInputActionValue& Value);
	void SelectPoint(const FInputActionValue& Value);
	void Rise(const FInputActionValue& Value);
	void ProcessInput(float Deltatime);

	void GenerateBuilding(int SizeX, int SizeY, int SizeZ, const FVector& Location, const FRotator& Rotation);
	int FindVertex(const FVector& Target, UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID);
	int FindLowestVertex(UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID);

	void DrawVectorDebugArrows(UStaticMeshComponent* MeshComponent, const FVector& Acceleration);
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
	TObjectPtr<UInputAction> SelectPointAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> RiseAction;

	//Control
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

	//WFC
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AWFCGenerator> WFCGenerator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VertexSelectionTolerance = 500.f;

	//Scan
	TArray<FVector> SelectedGridVertices;
};
