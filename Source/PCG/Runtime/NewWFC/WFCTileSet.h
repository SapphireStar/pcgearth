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
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TileSetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UDataTable> TileSetTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FWFCTileRuleSet> TileRuleSets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FWFCSocketRuleSet> SocketRuleSets;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FWFCTileDefinition> Tiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FWFCSocket> SocketDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FWFCConfiguration DefaultConfiguration;

public:
    UFUNCTION(BlueprintCallable, BlueprintPure)
    int32 GetTileCount() const { return Tiles.Num(); }

    UFUNCTION(BlueprintCallable, BlueprintPure)
    FWFCTileDefinition GetTile(int32 Index) const;

    UFUNCTION(BlueprintCallable, BlueprintPure)
    int32 FindTileByName(const FString& TileName) const;

    UFUNCTION(BlueprintCallable, BlueprintPure)
    TArray<int32> GetTilesByCategory(EWFCTileCategory Category) const;

    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool AreSocketsCompatible(const FString& Socket1, const FString& Socket2) const;

    UFUNCTION(BlueprintCallable, BlueprintPure)
    FWFCSocket GetSocketDefinition(const FString& SocketName) const;

    UFUNCTION(BlueprintCallable)
    bool ValidateTileSet(FString& OutErrorMessage) const;

    UFUNCTION(BlueprintCallable, CallInEditor)
    void ReadDatatable();
    
    UFUNCTION(BlueprintCallable, CallInEditor)
    void GenerateRotationVariants();

protected:
    FString RotateSocketName(const FString& SocketName, int32 RotationSteps) const;
    
    TArray<FString> RotateSockets(const TArray<FString>& OriginalSockets, int32 RotationSteps) const;

    bool HasSocket(const FString& SocketName, int32& outIndex) const;
};

