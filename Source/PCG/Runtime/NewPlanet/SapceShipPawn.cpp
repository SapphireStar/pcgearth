#include "SapceShipPawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

ASpaceshipPawn::ASpaceshipPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // 创建根网格组件（用于物理）
    RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
    RootComponent = RootMesh;
    
    // 创建球形碰撞组件
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetupAttachment(RootMesh);
    CollisionComponent->SetSphereRadius(75.0f);
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);

    // 创建飞船网格组件
    ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
    ShipMesh->SetupAttachment(RootMesh);
    ShipMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 初始化变量
    CurrentGravityDirection = FVector(0, 0, -1);
    TargetGravityDirection = CurrentGravityDirection;
    PlanetCenter = FVector::ZeroVector;
    bHasValidSurface = false;
    LastValidSurfaceNormal = FVector(0, 0, 1);
    LastSurfaceDistance = 0.0f;
    
    MovementInput = FVector::ZeroVector;
    RotationInput = FVector::ZeroVector;
}

void ASpaceshipPawn::BeginPlay()
{
    Super::BeginPlay();
    InitializePhysics();
}

void ASpaceshipPawn::InitializePhysics()
{
    if (RootMesh)
    {
        // 启用物理模拟
        RootMesh->SetSimulatePhysics(true);
        RootMesh->SetEnableGravity(false); // 我们自己处理重力
        
        // 设置物理属性
        RootMesh->SetLinearDamping(LinearDamping);
        RootMesh->SetAngularDamping(AngularDamping);
        
        // 设置质量
        RootMesh->SetMassOverrideInKg(NAME_None, 1000.0f);
        
        // 限制角速度
        //RootMesh->GetBodyInstance()->SetMaxAngularVelocity(MaxAngularVelocity, false);
        
        // 设置重心
        RootMesh->GetBodyInstance()->COMNudge = FVector(0, 0, 0);
    }
}

void ASpaceshipPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!RootMesh || !RootMesh->IsSimulatingPhysics())
        return;

    // 更新重力方向
    FVector DesiredGravity = GetDesiredGravityDirection();
    if (!DesiredGravity.IsZero())
    {
        TargetGravityDirection = DesiredGravity;
    }

    // 平滑过渡重力方向
    if (!CurrentGravityDirection.Equals(TargetGravityDirection, 0.01f))
    {
        CurrentGravityDirection = FMath::VInterpTo(CurrentGravityDirection, TargetGravityDirection, 
                                                 DeltaTime, GravityTransitionSpeed);
        CurrentGravityDirection.Normalize();
    }

    // 应用各种力
    ApplyGravity(DeltaTime);
    ApplyMovementForces(DeltaTime);
    ApplyHoverForce(DeltaTime);
    
    if (bAlignToSurface)
    {
        ApplySurfaceAlignment(DeltaTime);
    }
    
    if (bAutoStabilize)
    {
        ApplyStabilization(DeltaTime);
    }
    
    // 限制角速度
    LimitAngularVelocity();

    // 重置输入
    MovementInput = FVector::ZeroVector;
    RotationInput = FVector::ZeroVector;

    // 调试绘制
    if (GEngine && GEngine->bEnableOnScreenDebugMessages)
    {
        FVector CurrentLocation = GetActorLocation();
        DrawDebugLine(GetWorld(), CurrentLocation, 
                     CurrentLocation + CurrentGravityDirection * 300.0f, 
                     FColor::Red, false, -1.0f, 0, 3.0f);
        
        // 绘制表面检测
        FVector HitLocation, HitNormal;
        float Distance;
        if (DetectSurface(HitLocation, HitNormal, Distance))
        {
            DrawDebugLine(GetWorld(), CurrentLocation, HitLocation, FColor::Green, false, -1.0f, 0, 2.0f);
            DrawDebugLine(GetWorld(), HitLocation, HitLocation + HitNormal * 200.0f, FColor::Blue, false, -1.0f, 0, 2.0f);
        }
    }
}

void ASpaceshipPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // 移动输入
    PlayerInputComponent->BindAxis("MoveForward", this, &ASpaceshipPawn::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ASpaceshipPawn::MoveRight);
    PlayerInputComponent->BindAxis("MoveUp", this, &ASpaceshipPawn::MoveUp);

    // 旋转输入
    PlayerInputComponent->BindAxis("Turn", this, &ASpaceshipPawn::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &ASpaceshipPawn::LookUp);
    PlayerInputComponent->BindAxis("Roll", this, &ASpaceshipPawn::Roll);
}

void ASpaceshipPawn::MoveForward(float Value)
{
    MovementInput.X = FMath::Clamp(Value, -1.0f, 1.0f);
}

void ASpaceshipPawn::MoveRight(float Value)
{
    MovementInput.Y = FMath::Clamp(Value, -1.0f, 1.0f);
}

void ASpaceshipPawn::MoveUp(float Value)
{
    MovementInput.Z = FMath::Clamp(Value, -1.0f, 1.0f);
}

void ASpaceshipPawn::Turn(float Value)
{
    RotationInput.Z = Value; // Yaw
}

void ASpaceshipPawn::LookUp(float Value)
{
    RotationInput.Y = Value; // Pitch
}

void ASpaceshipPawn::Roll(float Value)
{
    RotationInput.X = Value; // Roll
}

void ASpaceshipPawn::ApplyGravity(float DeltaTime)
{
    if (!RootMesh)
        return;

    FVector GravityForce = CurrentGravityDirection * GravityStrength * RootMesh->GetMass();
    RootMesh->AddForce(GravityForce);
}

void ASpaceshipPawn::ApplyMovementForces(float DeltaTime)
{
    if (!RootMesh || MovementInput.IsZero())
        return;

    // 获取当前速度
    FVector CurrentVelocity = RootMesh->GetPhysicsLinearVelocity();
    
    // 检查是否超过最大速度
    if (CurrentVelocity.Size() > MaxSpeed)
    {
        // 只允许减速或垂直于当前速度的移动
        FVector VelocityDirection = CurrentVelocity.GetSafeNormal();
        FVector LocalForward = GetActorForwardVector();
        FVector LocalRight = GetActorRightVector();
        FVector LocalUp = GetActorUpVector();
        
        // 计算移动方向
        FVector MoveDirection = (LocalForward * MovementInput.X) + 
                               (LocalRight * MovementInput.Y) + 
                               (LocalUp * MovementInput.Z);
        
        // 只允许与当前速度方向相反或垂直的移动
        float DotProduct = FVector::DotProduct(MoveDirection, VelocityDirection);
        if (DotProduct > 0.1f)
        {
            // 限制前进力
            MoveDirection = MoveDirection - (VelocityDirection * DotProduct);
        }
        
        FVector ThrustVector = MoveDirection * ThrustForce;
        RootMesh->AddForce(ThrustVector);
    }
    else
    {
        // 正常推力
        FVector LocalForward = GetActorForwardVector();
        FVector LocalRight = GetActorRightVector();
        FVector LocalUp = GetActorUpVector();
        
        FVector ThrustVector = (LocalForward * MovementInput.X * ThrustForce) + 
                              (LocalRight * MovementInput.Y * ThrustForce) + 
                              (LocalUp * MovementInput.Z * ThrustForce);
        
        RootMesh->AddForce(ThrustVector);
    }

    // 应用旋转力矩
    if (!RotationInput.IsZero())
    {
        FVector Torque = FVector(RotationInput.X * RollTorque,
                                RotationInput.Y * PitchTorque,
                                RotationInput.Z * TurnTorque);
        
        RootMesh->AddTorqueInRadians(Torque);
    }
}

void ASpaceshipPawn::ApplyHoverForce(float DeltaTime)
{
    if (!RootMesh)
        return;

    FVector HitLocation, HitNormal;
    float Distance;
    
    if (DetectSurface(HitLocation, HitNormal, Distance))
    {
        LastSurfaceDistance = Distance;
        
        if (Distance < HoverHeight)
        {
            // 计算悬停力
            float HoverRatio = 1.0f - (Distance / HoverHeight);
            FVector HoverForceVector = -CurrentGravityDirection * HoverForce * HoverRatio;
            
            // 添加阻尼以减少震荡
            FVector Velocity = RootMesh->GetPhysicsLinearVelocity();
            FVector VerticalVelocity = FVector::DotProduct(Velocity, -CurrentGravityDirection) * (-CurrentGravityDirection);
            FVector DampingForce = -VerticalVelocity * HoverForce * 0.5f;
            
            RootMesh->AddForce(HoverForceVector + DampingForce);
        }
    }
}

void ASpaceshipPawn::ApplySurfaceAlignment(float DeltaTime)
{
    if (!RootMesh)
        return;

    FVector HitLocation, HitNormal;
    float Distance;
    
    if (DetectSurface(HitLocation, HitNormal, Distance))
    {
        // 计算目标旋转
        FQuat TargetRotation = GetSurfaceAlignmentRotation(HitNormal);
        FQuat CurrentRotation = GetActorQuat();
        
        // 计算角度差
        float AngleDifference = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(
            CurrentRotation.GetUpVector(), HitNormal), -1.0f, 1.0f)));
        
        if (AngleDifference < MaxSurfaceAlignmentAngle)
        {
            // 计算需要的角速度
            FQuat DeltaRotation = TargetRotation * CurrentRotation.Inverse();
            FVector RotationAxis;
            float RotationAngle;
            DeltaRotation.ToAxisAndAngle(RotationAxis, RotationAngle);
            
            // 转换为力矩
            FVector AlignmentTorque = RotationAxis * RotationAngle * SurfaceAlignmentStrength;
            
            // 添加阻尼
            FVector AngularVelocity = RootMesh->GetPhysicsAngularVelocityInRadians();
            FVector DampingTorque = -AngularVelocity * SurfaceAlignmentStrength * 0.3f;
            
            RootMesh->AddTorqueInRadians(AlignmentTorque + DampingTorque);
        }
    }
}

void ASpaceshipPawn::ApplyStabilization(float DeltaTime)
{
    if (!RootMesh)
        return;

    FVector AngularVelocity = RootMesh->GetPhysicsAngularVelocityInRadians();
    
    // 只有在没有旋转输入时才应用稳定化
    if (RotationInput.IsNearlyZero() && AngularVelocity.Size() > StabilityThreshold)
    {
        FVector StabilizationTorque = -AngularVelocity * StabilityForce;
        RootMesh->AddTorqueInRadians(StabilizationTorque);
    }
}

FVector ASpaceshipPawn::GetDesiredGravityDirection() const
{
    if (bUseSphericalGravity)
    {
        // 球形重力：指向星球中心
        FVector CurrentLocation = GetActorLocation();
        FVector ToPlanet = PlanetCenter - CurrentLocation;
        float DistanceToPlanet = ToPlanet.Size();
        
        if (DistanceToPlanet > 0.0f)
        {
            return ToPlanet / DistanceToPlanet;
        }
    }
    else
    {
        // 表面重力：基于表面法线
        FVector HitLocation, HitNormal;
        float Distance;
        if (DetectSurface(HitLocation, HitNormal, Distance))
        {
            return -HitNormal;
        }
    }

    return FVector(0, 0, -1);
}

bool ASpaceshipPawn::DetectSurface(FVector& OutHitLocation, FVector& OutHitNormal, float& OutDistance) const
{
    if (!GetWorld())
        return false;

    FVector CurrentLocation = GetActorLocation();
    FVector TraceStart = CurrentLocation;
    FVector TraceEnd;

    if (bUseSphericalGravity)
    {
        FVector ToPlanet = (PlanetCenter - CurrentLocation).GetSafeNormal();
        TraceEnd = TraceStart + ToPlanet * SurfaceDetectionDistance;
    }
    else
    {
        TraceEnd = TraceStart + CurrentGravityDirection * SurfaceDetectionDistance;
    }

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
        OutDistance = (HitResult.Location - CurrentLocation).Size();
        return true;
    }

    return false;
}

FVector ASpaceshipPawn::GetLocalUpVector() const
{
    return GetActorUpVector();
}

FQuat ASpaceshipPawn::GetSurfaceAlignmentRotation(const FVector& SurfaceNormal) const
{
    FVector CurrentForward = GetActorForwardVector();
    FVector CurrentRight = GetActorRightVector();
    
    // 计算新的up向量（与表面法线对齐）
    FVector NewUp = SurfaceNormal;
    
    // 计算新的right向量（保持在表面平面内）
    FVector NewRight = FVector::CrossProduct(CurrentForward, NewUp).GetSafeNormal();
    if (NewRight.IsNearlyZero())
    {
        NewRight = FVector::CrossProduct(CurrentRight, NewUp).GetSafeNormal();
    }
    
    // 计算新的forward向量
    FVector NewForward = FVector::CrossProduct(NewUp, NewRight).GetSafeNormal();
    
    return FQuat(FMatrix(NewForward, NewRight, NewUp, FVector::ZeroVector));
}

void ASpaceshipPawn::LimitAngularVelocity()
{
    if (!RootMesh)
        return;

    FVector AngularVelocity = RootMesh->GetPhysicsAngularVelocityInRadians();
    if (AngularVelocity.Size() > MaxAngularVelocity)
    {
        FVector LimitedAngularVelocity = AngularVelocity.GetSafeNormal() * MaxAngularVelocity;
        RootMesh->SetPhysicsAngularVelocityInRadians(LimitedAngularVelocity);
    }
}