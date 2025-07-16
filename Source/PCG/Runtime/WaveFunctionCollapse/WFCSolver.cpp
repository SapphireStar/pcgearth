// Fill out your copyright notice in the Description page of Project Settings.


#include "WFCSolver.h"

#include "../../../../../../../../Shared/Epic Games/UE_5.5/Engine/Plugins/Media/AvfMedia/Source/AvfMediaCapture/Private/Player/AvfMediaCaptureHelper.h"

AWFCSolver::AWFCSolver(int sizeX, int sizeY, int sizeZ, int tileNum, bool periodic,
                       const TArray<TArray<TArray<int>>>& propagator, const TArray<double>& weights)
{
	this->SizeX = sizeX;
	this->SizeY = sizeY;
	this->SizeZ = sizeZ;
	this->TileNum = tileNum;
	this->Periodic = periodic;
	
	Propagator.SetNum(6);
	for (int d = 0; d < 6; d++)
	{
		Propagator[d].SetNum(propagator[d].Num());
		for (int t = 0; t < TileNum; t++)
		{
			for (int othert = 0; othert < propagator[d][t].Num(); othert++)
			{
				Propagator[d][t].Add(propagator[d][t][othert]);
			}
		}
	}

	Weights.SetNum(weights.Num());
	for (int i = 0; i < weights.Num(); i++)
	{
		Weights[i] = weights[i];
	}

	Init(sizeX, sizeY, sizeZ, periodic);
}

/*void AWFCSolver::TestInit()
{
	Weights = TArray<double>{{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}};
	TileNum = 6;

	Propagator.SetNum(6);
	for (int d = 0; d < 6; d++)
	{
		Propagator[d].SetNum(TileNum);
		for (int t = 0; t < TileNum; t++)
		{
			for (int othert = 0; othert < TileNum; othert++)
			{
				if (othert != t)
					Propagator[d][t].Add(othert);
			}
		}
	}
	Init(5, 5, 5, false);
}*/

void AWFCSolver::Init(int width, int length, int height, bool periodic)
{
	SizeX = width;
	SizeY = length;
	SizeZ = height;
	this->Periodic = periodic;

	WaveState.SetNum(SizeX * SizeY * SizeZ);
	Compatible.SetNum(WaveState.Num());
	for (int i = 0; i< WaveState.Num(); ++i)
	{
		WaveState[i].SetNum(TileNum);
		Compatible[i].SetNum(TileNum);
		for (int j = 0; j < TileNum; ++j)
		{
			Compatible[i][j].SetNum(6);
		}
	}

	Distribution.SetNum(TileNum);
	Observed.SetNum(SizeX * SizeY *  SizeZ);

	WeightLogWeights.SetNum(TileNum);
	SumOfWeights = 0;
	SumOfWeightLogWeights = 0;

	for (int t = 0; t < TileNum; ++t)
	{
		WeightLogWeights[t] = Weights[t] * FMath::Log2(Weights[t]);
		SumOfWeights += Weights[t];
		SumOfWeightLogWeights += WeightLogWeights[t];
	}

	StartingEntropy = FMath::Log2(SumOfWeights) - SumOfWeightLogWeights / SumOfWeights;

	SumsOfOnes.SetNum(SizeX * SizeY * SizeZ);
	SumsOfWeights.SetNum(SizeX * SizeY * SizeZ);
	SumsOfWeightLogWeights.SetNum(SizeX * SizeY * SizeZ);
	Entropies.SetNum(SizeX * SizeY * SizeZ);

	BannedStateStack.SetNum(WaveState.Num() * TileNum);
	StackSize = 0;
}

bool AWFCSolver::Run(int seed, int limit, TArray<int>& outResult)
{
	if (WaveState.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No wave state found"));
		return false;
	}
	
	Clear();
	FRandomStream RandomGenerator(seed);

	for (int l = 0; l < limit; ++l)
	{
		int node = NextUnobservedNode(RandomGenerator);
		if (node >= 0)
		{
			Observe(node, RandomGenerator);
			bool success = Propagate();
			if (!success) return false;
		}
		else
		{
			outResult.SetNum(SizeX * SizeY * SizeZ);
			for (int i = 0; i < WaveState.Num(); ++i)
			{
				for (int t = 0; t < TileNum; ++t)
				{
					if (WaveState[i][t])
					{
						//完成坍缩，输出结果
						Observed[i] = t;
						outResult[i] = t;
						break;
					}
				}
			}
			return true;
		}
	}

	outResult.SetNum(SizeX * SizeY * SizeZ);
	for (int i = 0; i < WaveState.Num(); ++i)
	{
		for (int t = 0; t < TileNum; ++t)
		{
			if (WaveState[i][t])
			{
				//完成坍缩，输出结果
				outResult[i] = t;
				break;
			}
		}
	}
	return true;
	
}

int AWFCSolver::NextUnobservedNode(FRandomStream& random)
{
	double min = DBL_MAX;
	int nodeMinEntropy = -1;
	for (int i = 0; i < WaveState.Num(); ++i)
	{
		if (!Periodic && (i % SizeZ + N > SizeZ || (i/SizeZ)%SizeY + N > SizeY || ((i/SizeZ)/SizeY) % SizeX + N > SizeX))
			continue;
		int remainingValues = SumsOfOnes[i];
		double entropy = Entropies[i];
		if (remainingValues > 1 && entropy <= min)
		{
			double noise = 1E-6 * random.FRand();
			nodeMinEntropy = i;
		}
			
	}
	return nodeMinEntropy;
}

void AWFCSolver::Observe(int node, FRandomStream& random)
{
	TArray<bool>& w = WaveState[node];
	
	for (int t = 0; t < TileNum; ++t)
	{
		Distribution[t] = w[t] ? Weights[t] : 0.f;
	}
	int RandomTile = RandomChooseTile(Distribution, random.FRand());
	for (int t = 0; t < TileNum; ++t)
	{
		if (w[t] != (t==RandomTile))
		{
			Ban(node, t);
		}
	}
}

bool AWFCSolver::Propagate()
{
	while (StackSize > 0)
	{
		TPair<int, int> pair = BannedStateStack[StackSize-1];
		StackSize--;
		int nodeBanned = pair.Key;
		int nodeBannedTile = pair.Value;

		int nodeBannedX = ((nodeBanned / SizeZ) / SizeY) % SizeX;
		int nodeBannedY = (nodeBanned / SizeZ) % SizeY;
		int nodeBannedZ = nodeBanned % SizeZ;

		for (int d = 0; d < 6; d++)
		{
			int NodeNeighbourX = nodeBannedX + dx[d];
			int NodeNeighbourY = nodeBannedY + dy[d];
			int NodeNeighbourZ = nodeBannedZ + dz[d];
			if (!Periodic && (NodeNeighbourX < 0 || NodeNeighbourY <0 || NodeNeighbourZ < 0 ||
				NodeNeighbourX + N >  SizeX || NodeNeighbourY + N > SizeY ||  NodeNeighbourZ + N > SizeZ))
			{
				continue;
			}

			if (NodeNeighbourX < 0) NodeNeighbourX += SizeX;
			else if (NodeNeighbourX >= SizeX) NodeNeighbourX -= SizeX;
			if (NodeNeighbourY < 0) NodeNeighbourY += SizeY;
			else if (NodeNeighbourY >= SizeY) NodeNeighbourY -= SizeY;
			if (NodeNeighbourZ < 0) NodeNeighbourZ += SizeZ;
			else if (NodeNeighbourZ >= SizeZ) NodeNeighbourZ -= SizeZ;

			int NodeNeighbour = NodeNeighbourZ + NodeNeighbourY * SizeZ + NodeNeighbourX * SizeY * SizeZ;
			TArray<int>& p = Propagator[d][nodeBannedTile];
			TArray<TArray<int>>& compat = Compatible[NodeNeighbour];

			for (int l = 0; l < p.Num(); l++)
			{
				int NodeNeighbourTile = p[l];
				TArray<int>& comp = compat[NodeNeighbourTile];

				comp[d]--;
				if (comp[d] == 0)
				{
					Ban(NodeNeighbour, NodeNeighbourTile);
				}
			}
		}
	}
	return SumsOfOnes[0] > 0;
}

void AWFCSolver::Ban(int node, int tile)
{
	WaveState[node][tile] = false;

	TArray<int>& comp = Compatible[node][tile];

	for (int d = 0; d < 6; d++)
	{
		comp[d] = 0;
	}
	BannedStateStack[StackSize] = TPair<int, int>(node, tile);
	StackSize++;

	SumsOfOnes[node] -= 1;
	SumsOfWeights[node] -= Weights[tile];
	SumsOfWeightLogWeights[node] -= WeightLogWeights[tile];

	double sum = SumsOfWeights[node];
	Entropies[node] = FMath::Log2(sum) - SumsOfWeightLogWeights[node] / sum;
}


void AWFCSolver::Clear()
{
	for (int i = 0; i < WaveState.Num(); i++)
	{
		for (int t = 0; t < TileNum; t++)
		{
			WaveState[i][t] = true;
			for (int d = 0; d< 6; d++)
			{
				Compatible[i][t][d] = Propagator[opposite[d]][t].Num();
			}
		}

		SumsOfOnes[i] = Weights.Num();
		SumsOfWeights[i] = SumOfWeights;
		SumsOfWeightLogWeights[i] = SumOfWeightLogWeights;
		Entropies[i] = StartingEntropy;
		Observed[i] = -1;
	}
}

int AWFCSolver::RandomChooseTile(const TArray<double>& weights, double rand)
{
	double sum = 0;
	for (int i = 0; i < weights.Num(); i++)
	{
		sum += weights[i];
	}
		
	double threshold = rand * sum;
	double partialSum = 0;
	for (int i = 0; i < weights.Num(); i++)
	{
		partialSum += weights[i];
		if (partialSum >= threshold) return i;
	}
	return 0;
}



