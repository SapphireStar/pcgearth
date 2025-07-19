// WFCPropagatorDataGenerator.h - Improved header with validation methods

#pragma once

#include "CoreMinimal.h"
#include "WFCTileData.h"
#include "Engine/DataAsset.h"
#include "WFCPropagatorDataGenerator.generated.h"

class UWFCSocketCompatibilityData;

USTRUCT(Blueprintable, BlueprintType)
struct FWFCRuleDataSettings
{
	GENERATED_BODY()
};

class PCG_API UWFCPropagatorDataGenerator
{
public:
	TObjectPtr<UWFCSocketCompatibilityData> SocketData;
	TObjectPtr<UWFCTileData> BaseTileData;
	TObjectPtr<UWFCTileData> CompleteTileData;
	TArray<TArray<TArray<int>>> Propagator;

public:
	UWFCPropagatorDataGenerator(TObjectPtr<UWFCSocketCompatibilityData> socketData, 
		TObjectPtr<UWFCTileData> baseTileData);
	
	bool GeneratePropagatorRules(TArray<TArray<TArray<int>>>& outPropagator);
	int GetTileCount() const;
	const FWFCTile GetTile(int TileID) const;
	UWFCTileData* GetCompleteData() const;

private:
	void GenerateCompleteData();
	bool ShouldCreateRotationVariants(const FWFCTile& BaseConfig);
	TArray<FString> RotateSocketsZ90(const TArray<FString>& OriginalSockets);
	
	// 生成Propagator规则表
	void BuildPropagatorTable();
	
	// 验证传播器规则的完整性和对称性
	void ValidatePropagatorRules();
	
	// 获取相对方向（用于约束匹配）
	int GetOppositeDirection(int Direction);
};