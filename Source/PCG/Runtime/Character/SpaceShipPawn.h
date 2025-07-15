// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "GameFramework/DefaultPawn.h"
#include "PCG/Runtime/WaveFunctionCollapse/WFCGenerator.h"
#include "SpaceShipPawn.generated.h"

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
	void Look(const FInputActionValue& Value);
	void Roll(const FInputActionValue& Value);
	void SelectPoint(const FInputActionValue& Value);
	void Rise(const FInputActionValue& Value);

	void GenerateBuilding(int SizeX, int SizeY, int SizeZ, const FVector& Location, const FRotator& Rotation);

protected:
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

	//WFC
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AWFCGenerator> WFCGenerator;
};
