#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "SapceShipPawn.generated.h"

UCLASS()
class PCG_API ASpaceshipPawn : public APawn
{
    GENERATED_BODY()

public:
    ASpaceshipPawn();

protected:
    virtual void BeginPlay() override;

    // 组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* CollisionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* ShipMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* RootMesh;

    // 飞船移动参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship Movement")
    float ThrustForce = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship Movement")
    float MaxSpeed = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship Movement")
    float LinearDamping = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship Movement")
    float AngularDamping = 5.0f;

    // 旋转参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship Rotation")
    float TurnTorque = 80000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship Rotation")
    float PitchTorque = 80000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship Rotation")
    float RollTorque = 60000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship Rotation")
    float MaxAngularVelocity = 3.0f;

    // 重力和地面检测参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
    float GravityStrength = 980.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
    float SurfaceDetectionDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
    float HoverHeight = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
    float HoverForce = 100000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
    float GravityTransitionSpeed = 2.0f;

    // 稳定系统参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stability")
    float StabilityForce = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stability")
    bool bAutoStabilize = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stability")
    float StabilityThreshold = 0.1f;

    // 当前重力方向
    UPROPERTY(BlueprintReadOnly, Category = "Gravity")
    FVector CurrentGravityDirection;

    // 星球参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
    FVector PlanetCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
    float PlanetRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
    bool bUseSphericalGravity = true;

    // 地面对齐参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Alignment")
    bool bAlignToSurface = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Alignment")
    float SurfaceAlignmentStrength = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Alignment")
    float MaxSurfaceAlignmentAngle = 45.0f;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // 输入函数
    void MoveForward(float Value);
    void MoveRight(float Value);
    void MoveUp(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void Roll(float Value);

    // 重力和表面检测函数
    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void ApplyGravity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Gravity")
    FVector GetDesiredGravityDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Surface")
    bool DetectSurface(FVector& OutHitLocation, FVector& OutHitNormal, float& OutDistance) const;

    UFUNCTION(BlueprintCallable, Category = "Surface")
    void ApplySurfaceAlignment(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Surface")
    void ApplyHoverForce(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ApplyMovementForces(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ApplyStabilization(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Planet")
    void SetPlanetCenter(FVector NewCenter) { PlanetCenter = NewCenter; }

    // 获取物理体组件
    UFUNCTION(BlueprintCallable, Category = "Physics")
    UPrimitiveComponent* GetPhysicsComponent() const { return RootMesh; }

private:
    // 输入变量
    FVector MovementInput;
    FVector RotationInput;
    
    // 重力相关
    FVector TargetGravityDirection;
    FVector LastValidSurfaceNormal;
    bool bHasValidSurface;
    float LastSurfaceDistance;
    
    // 内部辅助函数
    void InitializePhysics();
    FVector GetLocalUpVector() const;
    FQuat GetSurfaceAlignmentRotation(const FVector& SurfaceNormal) const;
    void LimitAngularVelocity();
};