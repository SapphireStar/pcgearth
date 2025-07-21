// WFCTypes.h - 核心类型定义
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WFCTypes.generated.h"

// 基础枚举定义
UENUM(BlueprintType)
enum class EWFCDirection : uint8
{
    Up = 0      UMETA(DisplayName = "Up (+Z)"),
    Down = 1    UMETA(DisplayName = "Down (-Z)"),
    North = 2   UMETA(DisplayName = "North (+Y)"),
    South = 3   UMETA(DisplayName = "South (-Y)"),
    East = 4    UMETA(DisplayName = "East (+X)"),
    West = 5    UMETA(DisplayName = "West (-X)")
};

UENUM(BlueprintType)
enum class EWFCTileCategory : uint8
{
    Unknown = 0     UMETA(DisplayName = "Unknown"),
    Ground = 1      UMETA(DisplayName = "Ground/Floor"),
    Wall = 2        UMETA(DisplayName = "Wall"),
    Ceiling = 3     UMETA(DisplayName = "Ceiling"),
    Structure = 4   UMETA(DisplayName = "Structure"),
    Decoration = 5  UMETA(DisplayName = "Decoration"),
    Empty = 6       UMETA(DisplayName = "Empty")
};

UENUM(BlueprintType)
enum class EWFCTileDecorator : uint8
{
    None = 0                        UMETA(DisplayName = "None"),
    MustHorizontalConnected = 1     UMETA(DisplayName = "MustHorizontalConnected"),
};

UENUM(BlueprintType)
enum class EWFCGenerationMode : uint8
{
    Random = 0          UMETA(DisplayName = "Random Order"),
    GroundFirst = 1     UMETA(DisplayName = "Ground First"),
    LayeredBottomUp = 2 UMETA(DisplayName = "Bottom-Up Layers"),
    CenterOutward = 3   UMETA(DisplayName = "Center Outward"),
    Custom = 99         UMETA(DisplayName = "Custom Order")
};

// 坐标结构
USTRUCT(BlueprintType)
struct FWFCCoordinate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 X = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Y = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Z = 0;

    FWFCCoordinate() = default;
    FWFCCoordinate(int32 InX, int32 InY, int32 InZ) : X(InX), Y(InY), Z(InZ) {}

    bool operator==(const FWFCCoordinate& Other) const
    {
        return X == Other.X && Y == Other.Y && Z == Other.Z;
    }

    friend uint32 GetTypeHash(const FWFCCoordinate& Coord)
    {
        return HashCombine(HashCombine(GetTypeHash(Coord.X), GetTypeHash(Coord.Y)), GetTypeHash(Coord.Z));
    }

    FString ToString() const
    {
        return FString::Printf(TEXT("(%d, %d, %d)"), X, Y, Z);
    }
};

// Socket定义
USTRUCT(BlueprintType)
struct FWFCSocket
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> CompatibleSockets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowEmpty = false;

    FWFCSocket() = default;
    FWFCSocket(const FString& Name) : SocketName(Name) {}
};

// 瓦片定义
USTRUCT(BlueprintType)
struct FWFCTileDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWFCTileCategory Category = EWFCTileCategory::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UMaterial> Material;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ArraySizeEnum = "EWFCDirection"))
    TArray<FString> Sockets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.1, ClampMax = 10.0))
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanRotate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator BaseRotation = FRotator::ZeroRotator;

    // 约束条件
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxInstancesPerGeneration = -1; // -1表示无限制

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresSupport = false; // 是否需要下方支撑

    FWFCTileDefinition()
    {
        Sockets.SetNum(6); // 确保有6个方向的socket
    }

    FString GetSocket(EWFCDirection Direction) const
    {
        int32 Index = static_cast<int32>(Direction);
        return (Index >= 0 && Index < Sockets.Num()) ? Sockets[Index] : TEXT("");
    }

    void SetSocket(EWFCDirection Direction, const FString& SocketName)
    {
        int32 Index = static_cast<int32>(Direction);
        if (Index >= 0 && Index < 6)
        {
            if (Sockets.Num() < 6) Sockets.SetNum(6);
            Sockets[Index] = SocketName;
        }
    }
};

USTRUCT(BlueprintType)
struct FWFCTileRuleSet
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TileRuleSetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FWFCTileDefinition> Tiles;
};

USTRUCT(BlueprintType)
struct FWFCSocketRuleSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SocketRuleSetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FWFCSocket> Sockets;
};

USTRUCT(BlueprintType)
struct FWFCGenerationConstraint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ConstraintName;

    // 位置约束
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FWFCCoordinate> RequiredPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FWFCCoordinate> ForbiddenPositions;

    // 瓦片约束
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> AllowedTileIndices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> ForbiddenTileIndices;

    // 层级约束
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinLayer = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxLayer = -1;

    // 数量约束
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinInstances = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxInstances = -1;
};

// WFC配置
USTRUCT(BlueprintType)
struct FWFCConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FIntVector GridSize = FIntVector(5, 5, 3);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPeriodicBoundary = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWFCGenerationMode GenerationMode = EWFCGenerationMode::Random;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxIterations = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RandomSeed = 12345;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableBacktracking = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BacktrackingDepth = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FWFCGenerationConstraint> Constraints;
};

// 生成结果
USTRUCT(BlueprintType)
struct FWFCGenerationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bSuccess = false;

    UPROPERTY(BlueprintReadOnly)
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly)
    int32 IterationsUsed = 0;

    UPROPERTY(BlueprintReadOnly)
    TMap<FWFCCoordinate, int32> TileAssignments;

    UPROPERTY(BlueprintReadOnly)
    TArray<FWFCCoordinate> FailedPositions;

    UPROPERTY(BlueprintReadOnly)
    float GenerationTimeSeconds = 0.0f;
};