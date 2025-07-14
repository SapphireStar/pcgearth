// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WFCSolver.generated.h"

UENUM(BlueprintType)
enum class ESocketDirection : uint8
{
	SE_Up = 0 UMETA(DisplayName = "Up"),
	SE_Down = 1 UMETA(DisplayName = "Down"),
	SE_Left = 2 UMETA(DisplayName = "Left"),
	SE_Right = 3 UMETA(DisplayName = "Right"),
	SE_Forward = 4 UMETA(DisplayName = "Forward"),
	SE_Backward = 5 UMETA(DisplayName = "Backward")
	
};

USTRUCT(Blueprintable)
struct FSocketInfo
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SocketUp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SocketDown;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SocketLeft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SocketRight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SocketForward;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SocketBackward;
};


class PCG_API AWFCSolver
{

public:
	AWFCSolver(int sizeX, int sizeY, int sizeZ, int tileNum, bool periodic, const TArray<TArray<TArray<int>>>& propagator, const TArray<double>& weights);
	void Init(int width, int length, int height, bool periodic);
	bool Run(int seed, int limit, TArray<int>& outResult);
	void Observe(int coord, FRandomStream& random);
	bool Propagate();
	void Ban(int node, int Tile);
	int NextUnobservedNode(FRandomStream& random);
	void Clear();
	int RandomChooseTile(const TArray<double>& weights, double rand);

protected:
	TArray<TArray<bool>> WaveState;

	TArray<TArray<TArray<int>>> Propagator;

	TArray<TArray<TArray<int>>> Compatible;

	TArray<int> Observed;

	TArray<TPair<int, int>> BannedStateStack;

	int StackSize;

	int SizeX;
	int SizeY;
	int SizeZ;
	int TileNum;
	bool Periodic;
	int N = 1;


	TArray<double> Weights;

	TArray<double> WeightLogWeights;

	TArray<double> Distribution;

	TArray<int> SumsOfOnes;

	double SumOfWeights, SumOfWeightLogWeights, StartingEntropy;
	TArray<double> SumsOfWeights;
	TArray<double> SumsOfWeightLogWeights;
	TArray<double> Entropies;

	TArray<int> dx{0, 0, 0, 0, 1, -1};
	TArray<int> dy{0, 0, -1, 1, 0, 0};
	TArray<int> dz{1, -1, 0, 0, 0, 0};
	TArray<int> opposite{1, 0, 3, 2, 5, 4};
};
