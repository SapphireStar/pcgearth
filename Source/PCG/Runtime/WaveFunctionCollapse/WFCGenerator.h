// WFCGenerator.h - 修改头文件以支持地板优先生成

#pragma once

#include "CoreMinimal.h"
#include "GridSystem.h"
#include "WFCSocketCompatibilityData.h"
#include "GameFramework/Actor.h"
#include "WFCTileData.h"
#include "WFCSolver.h" // 确保包含更新后的WFCSolver
#include "WFCGenerator.generated.h"

class UWFCTileData;
class UWFCPropagatorDataGenerator;

UCLASS()
class PCG_API AWFCGenerator : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AWFCGenerator();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // 原有的WFC启动方法（使用默认策略）
    UFUNCTION(BlueprintCallable, Category = "WaveFunction")
    void StartWFC(int SizeX, int SizeY, int SizeZ, const FVector& Position, const FRotator& Rotation, int Seed = 1000);

    // 新增：带策略的WFC启动方法
    UFUNCTION(BlueprintCallable, Category = "WaveFunction")
    void StartWFCWithStrategy(int SizeX, int SizeY, int SizeZ, const FVector& Position, const FRotator& Rotation, 
                             int Seed = 1000, EWFCGenerationStrategy Strategy = EWFCGenerationStrategy::FloorFirst);

    // 地板瓦片管理方法
    UFUNCTION(BlueprintCallable, Category = "WaveFunction|Floor")
    void SetFloorTileIndices(const TArray<int>& NewFloorIndices);
    
    UFUNCTION(BlueprintCallable, Category = "WaveFunction|Floor")
    void AddFloorTileIndex(int TileIndex);
    
    UFUNCTION(BlueprintCallable, Category = "WaveFunction|Floor")
    void RemoveFloorTileIndex(int TileIndex);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WaveFunction|Floor")
    TArray<FString> GetFloorTileNames() const;

    AGridSystem* CreateWFCGrid(int SizeX, int SizeY, int SizeZ, const FVector& Position, const FRotator& Rotation);
    AWFCBlock* CreateBlock(int t);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveFunction")
    bool Periodic;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveFunction")
    int N = 1;
    
    // 新增：生成策略选择
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveFunction|Strategy")
    EWFCGenerationStrategy GenerationStrategy;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveFunction")
    TObjectPtr<UWFCSocketCompatibilityData> SocketData;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveFunction")
    TObjectPtr<UWFCTileData> BaseTileData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveFunction")
    TObjectPtr<UWFCTileData> CompleteTileData;

    // 新增：地板瓦片索引数组
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveFunction|Floor", 
              meta = (ToolTip = "Indices of tiles that should be considered as floor tiles for floor-first generation"))
    TArray<int> FloorTileIndices;

    TArray<TObjectPtr<AGridSystem>> GridGenerator;
    TSharedPtr<UWFCPropagatorDataGenerator> PropagatorDataGenerator;
    TSharedPtr<AWFCSolver> WFCSolver;
    TArray<TArray<TArray<int>>> Propagator;
    TArray<double> Weights;
    FRandomStream RandomStream;

private:
    // 自动设置地板瓦片
    void SetupFloorTiles();
};