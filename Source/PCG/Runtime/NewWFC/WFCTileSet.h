#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WFCTypes.h"
#include "WFCTileSet.generated.h"

USTRUCT(BlueprintType)
struct PCG_API FWFCTileRuleSetRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Rule Set")
    FString RuleSetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Rule Set")
    TArray<FWFCTileDefinition> Tiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Rule Set")
    FString Description;

    FWFCTileRuleSetRow()
    {
        RuleSetName = TEXT("");
        Description = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct PCG_API FWFCSocketRuleSetRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Rule Set")
    FString RuleSetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Rule Set")
    TArray<FWFCSocket> Sockets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Rule Set")
    FString Description;

    FWFCSocketRuleSetRow()
    {
        RuleSetName = TEXT("");
        Description = TEXT("");
    }
};


UCLASS(BlueprintType)
class PCG_API UWFCTileSet : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Set")
    FString TileSetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FWFCTileRuleSet> TileRuleSets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FWFCSocketRuleSet> SocketRuleSets;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Set")
    TArray<FWFCTileDefinition> Tiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Rules")
    TArray<FWFCSocket> SocketDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    FWFCConfiguration DefaultConfiguration;

public:
    // 获取瓦片数量
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WFC")
    int32 GetTileCount() const { return Tiles.Num(); }

    // 根据索引获取瓦片
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WFC")
    FWFCTileDefinition GetTile(int32 Index) const;

    // 根据名称获取瓦片索引
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WFC")
    int32 FindTileByName(const FString& TileName) const;

    // 获取特定类别的瓦片
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WFC")
    TArray<int32> GetTilesByCategory(EWFCTileCategory Category) const;

    // Socket兼容性检查
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WFC")
    bool AreSocketsCompatible(const FString& Socket1, const FString& Socket2) const;

    // 获取Socket定义
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WFC")
    FWFCSocket GetSocketDefinition(const FString& SocketName) const;

    // 验证瓦片集合的完整性
    UFUNCTION(BlueprintCallable, Category = "WFC")
    bool ValidateTileSet(FString& OutErrorMessage) const;

    // 自动生成旋转变体
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "WFC")
    void GenerateRotationVariants();

protected:
    // 旋转Socket名称（Z轴90度）
    FString RotateSocketName(const FString& SocketName, int32 RotationSteps) const;
    
    // 旋转Socket数组
    TArray<FString> RotateSockets(const TArray<FString>& OriginalSockets, int32 RotationSteps) const;

    bool HasSocket(const FString& SocketName, int32& outIndex) const;
};

