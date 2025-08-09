// Fill out your copyright notice in the Description page of Project Settings.

#include "WFCCore.h"

#include "WFCPreProcessCache.h"

const TArray<FIntVector> FWFCCore::DirectionVectors = {
	FIntVector(0, 0, 1),
	FIntVector(0, 0, -1),
	FIntVector(0, 1, 0),
	FIntVector(0, -1, 0),
	FIntVector(1, 0, 0),
	FIntVector(-1, 0, 0)
};

FWFCCore::FWFCCore()
{
}

FWFCCore::~FWFCCore()
{
	Reset();
}

bool FWFCCore::Initialize(UWFCTileSet* InTileSet, const FWFCConfiguration& InConfig)
{
	if (!InTileSet)
	{
		return false;
	}

	FString ValidationError;
	//InTileSet->GenerateRotationVariants();
	if (!InTileSet->ValidateTileSet(ValidationError))
	{
		UE_LOG(LogTemp, Error, TEXT("WFCCore: TileSet validation failed: %s"), *ValidationError);
		return false;
	}

	if (InConfig.GridSize.X <= 0 || InConfig.GridSize.Y <= 0 || InConfig.GridSize.Z <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCCore: Invalid grid size: %s"), *InConfig.GridSize.ToString());
		return false;
	}

	TileSet = InTileSet;
	Config = InConfig;
	RandomGenerator.Initialize(Config.RandomSeed);

	Reset();

	InitializeGrid();
	BuildPropagationRules();
	//ApplyConstraints();

	return true;
}

void FWFCCore::UpdateGrid(const FWFCConfiguration& InConfig)
{
	Config.GridSize = InConfig.GridSize;
	InitializeGrid();
}

void FWFCCore::InitializeGrid()
{
	Grid.Empty();
	if (!TileSet)
	{
		return;
	}

	const int32 TileCount = TileSet->GetTileCount();
	const int32 TotalCells = Config.GridSize.X * Config.GridSize.Y * Config.GridSize.Z;
	

	for (int32 X = 0; X < Config.GridSize.X; X++)
	{
		for (int32 Y = 0; Y < Config.GridSize.Y; Y++)
		{
			for (int32 Z = 0; Z < Config.GridSize.Z; Z++)
			{
				FWFCCoordinate Coord(X, Y, Z);
				FWFCCell& Cell = Grid.Add(Coord, FWFCCell(TileCount));

				Cell.PossibleTiles.SetRange(0, TileCount, true);
				Cell.Entropy = CalculateEntropy(Cell);
			}
		}
	}
}

void FWFCCore::BuildPropagationRules()
{
	const int32 TileCount = TileSet->GetTileCount();
	PropagationRules.Empty();
	PropagationRules.SetNum(6);
	
	for (int32 Dir = 0; Dir < 6; Dir++)
	{
		PropagationRules[Dir].SetNum(TileCount);
		EWFCDirection Direction = static_cast<EWFCDirection>(Dir);
		EWFCDirection OppositeDirection = static_cast<EWFCDirection>(Dir ^ 1);

		const TCHAR* DirectionNames[] = {
			TEXT("Up"), TEXT("Down"), TEXT("Right"), TEXT("Left"), TEXT("Front"), TEXT("Back")
		};

		int32 TotalRules = 0;

		for (int32 TileA = 0; TileA < TileCount; TileA++)
		{
			FWFCTileDefinition TileDefA = TileSet->GetTile(TileA);
			FString SocketA = TileDefA.GetSocket(Direction);

			for (int32 TileB = 0; TileB < TileCount; TileB++)
			{
				FWFCTileDefinition TileDefB = TileSet->GetTile(TileB);
				FString SocketB = TileDefB.GetSocket(OppositeDirection);

				if (TileSet->AreSocketsCompatible(SocketA, SocketB))
				{
					PropagationRules[Dir][TileA].Add(TileB);
					TotalRules++;
					
				}
			}
		}
		
	}
	
	ValidatePropagationRules();
}

void FWFCCore::ValidatePropagationRules()
{
	bool bFoundAsymmetry = false;
	const int32 TileCount = TileSet->GetTileCount();

	for (int32 Dir = 0; Dir < 6; Dir++)
	{
		int32 OppositeDir = Dir ^ 1;

		for (int32 TileA = 0; TileA < TileCount; TileA++)
		{
			for (int32 TileB : PropagationRules[Dir][TileA])
			{
				if (!PropagationRules[OppositeDir][TileB].Contains(TileA))
				{
					bFoundAsymmetry = true;
				}
			}
		}
	}

	if (!bFoundAsymmetry)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Propagation Rules available"));
	}
}

void FWFCCore::CellPreProcess()
{
	//在预处理中加载缓存
	if (LoadPreProcessedGrid())
	{
		return;
	}
	
	for (const auto& [Coord, Cell] : Grid)
	{
		if (IsBoundaryCoordinate(Coord))
		{
			CollapseCellTo(Coord, 0);
			PropagateConstraints();
		}
	}
}


FWFCGenerationResult FWFCCore::Generate()
{
	FWFCGenerationResult Result;
	double StartTime = FPlatformTime::Seconds();

	TileInstanceCounts.Empty();
	CollapseHistory.Empty();
	InitializeGrid();

	while (!PropagationQueue.IsEmpty())
	{
		FWFCCoordinate Dummy;
		PropagationQueue.Dequeue(Dummy);
	}

	CellPreProcess();

	Result.bSuccess = RunGenerationLoop();
	int count = 0;
	while (!Result.bSuccess)
	{
		if (count >= 100)
		{break;}
		TileInstanceCounts.Empty();
		CollapseHistory.Empty();
		InitializeGrid();

		while (!PropagationQueue.IsEmpty())
		{
			FWFCCoordinate Dummy;
			PropagationQueue.Dequeue(Dummy);
		}

		CellPreProcess();
		Result.bSuccess = RunGenerationLoop();
		count++;
	}
	if (Result.bSuccess)
	{
		for (const auto& Coord : CollapseHistory)
		{
			const auto& Cell = GetCell(Coord);
			if (Cell->IsCollapsed())
			{
				Result.TileAssignments.Add(Coord, Cell->CollapsedTileIndex);
			}
			else
			{
				Result.FailedPositions.Add((Coord));
			}
		}
		/*for (const auto& [Coord, Cell] : Grid)
		{
			if (Cell.IsCollapsed())
			{
				Result.TileAssignments.Add(Coord, Cell.CollapsedTileIndex);
			}
			else
			{
				Result.FailedPositions.Add(Coord);
			}
		}*/
	}
	else
	{

		for (const auto& [Coord, Cell] : Grid)
		{
			if (Cell.IsCollapsed())
			{
				Result.TileAssignments.Add(Coord, Cell.CollapsedTileIndex);
			}
			else
			{
				Result.FailedPositions.Add(Coord);
			}
		}
	}

	Result.GenerationTimeSeconds = FPlatformTime::Seconds() - StartTime;
	Result.IterationsUsed = CollapseHistory.Num();

	return Result;
}

bool FWFCCore::RunGenerationLoop()
{
	for (int32 Iteration = 0; Iteration < Config.MaxIterations; Iteration++)
	{
		FWFCCoordinate NextCoord = SelectNextCell();

		if (NextCoord == FWFCCoordinate(-1, -1, -1))
		{
			return true;
		}

		if (!CollapseCell(NextCoord))
		{

			return false;
		}

		if (!PropagateConstraints())
		{
			return false;
		}
	}

	return false;
}

FWFCCoordinate FWFCCore::SelectNextCell()
{
	switch (Config.GenerationMode)
	{
	case EWFCGenerationMode::GroundFirst:
		return SelectCellGroundFirst();
	case EWFCGenerationMode::LayeredBottomUp:
		return SelectCellLayered();
	case EWFCGenerationMode::CenterOutward:
		return SelectCellCenterOut();
	default:
		return SelectCellRandom();
	}
}

FWFCCoordinate FWFCCore::SelectCellRandom()
{
	float MinEntropy = FLT_MAX;
	TArray<FWFCCoordinate> Candidates;

	for (const auto& [Coord, Cell] : Grid)
	{
		if (!Cell.IsCollapsed() && Cell.GetPossibleTileCount() > 0)
		{
			if (Cell.Entropy < MinEntropy)
			{
				MinEntropy = Cell.Entropy;
				Candidates.Empty();
				Candidates.Add(Coord);
			}
			else if (FMath::IsNearlyEqual(Cell.Entropy, MinEntropy, 0.001f))
			{
				Candidates.Add(Coord);
			}
		}
	}

	if (Candidates.Num() > 0)
	{
		int32 SelectedIndex = RandomGenerator.RandRange(0, Candidates.Num() - 1);
		return Candidates[SelectedIndex];
	}

	return FWFCCoordinate(-1, -1, -1);
}

FWFCCoordinate FWFCCore::SelectCellGroundFirst()
{
	TArray<int32> GroundTiles = TileSet->GetTilesByCategory(EWFCTileCategory::Ground);

	float MinEntropy = FLT_MAX;
	bool FoundGroundCandidate = false;
	TArray<FWFCCoordinate> GroundCandidates;
	TArray<FWFCCoordinate> OtherCandidates;

	for (const auto& [Coord, Cell] : Grid)
	{
		if (Cell.IsCollapsed() || Cell.GetPossibleTileCount() == 0) continue;

		bool CanPlaceGround = false;
		for (int32 GroundTile : GroundTiles)
		{
			if (GroundTile < Cell.PossibleTiles.Num() && Cell.CanPlace(GroundTile))
			{
				CanPlaceGround = true;
				break;
			}
		}

		if (CanPlaceGround)
		{
			if (Cell.Entropy < MinEntropy || !FoundGroundCandidate)
			{
				if (Cell.Entropy < MinEntropy)
				{
					MinEntropy = Cell.Entropy;
					GroundCandidates.Empty();
				}
				GroundCandidates.Add(Coord);
				FoundGroundCandidate = true;
			}
			else if (FMath::IsNearlyEqual(Cell.Entropy, MinEntropy, 0.001f))
			{
				GroundCandidates.Add(Coord);
			}
		}
		else if (!FoundGroundCandidate)
		{
			if (Cell.Entropy < MinEntropy)
			{
				MinEntropy = Cell.Entropy;
				OtherCandidates.Empty();
				OtherCandidates.Add(Coord);
			}
			else if (FMath::IsNearlyEqual(Cell.Entropy, MinEntropy, 0.001f))
			{
				OtherCandidates.Add(Coord);
			}
		}
	}

	if (GroundCandidates.Num() > 0)
	{
		int32 SelectedIndex = RandomGenerator.RandRange(0, GroundCandidates.Num() - 1);
		return GroundCandidates[SelectedIndex];
	}

	if (OtherCandidates.Num() > 0)
	{
		int32 SelectedIndex = RandomGenerator.RandRange(0, OtherCandidates.Num() - 1);
		return OtherCandidates[SelectedIndex];
	}

	return FWFCCoordinate(-1, -1, -1);
}

FWFCCoordinate FWFCCore::SelectCellLayered()
{
	for (int32 Z = 0; Z < Config.GridSize.Z; Z++)
	{
		float MinEntropy = FLT_MAX;
		TArray<FWFCCoordinate> LayerCandidates;

		for (int32 X = 0; X < Config.GridSize.X ; X++)
		{
			for (int32 Y = 0; Y < Config.GridSize.Y; Y++)
			{
				FWFCCoordinate Coord(X, Y, Z);
				if (const FWFCCell* Cell = GetCell(Coord))
				{
					if (!Cell->IsCollapsed() && Cell->GetPossibleTileCount() > 0)
					{
						if (Cell->Entropy < MinEntropy)
						{
							MinEntropy = Cell->Entropy;
							LayerCandidates.Empty();
							LayerCandidates.Add(Coord);
						}
						else if (FMath::IsNearlyEqual(Cell->Entropy, MinEntropy, 0.001f))
						{
							LayerCandidates.Add(Coord);
						}
					}
				}
			}
		}

		if (LayerCandidates.Num() > 0)
		{
			int32 SelectedIndex = RandomGenerator.RandRange(0, LayerCandidates.Num() - 1);
			return LayerCandidates[SelectedIndex];
		}
	}

	return FWFCCoordinate(-1, -1, -1);
}

FWFCCoordinate FWFCCore::SelectCellCenterOut()
{
	FIntVector Center = Config.GridSize / 2;
	float MinEntropy = FLT_MAX;
	float MinDistance = FLT_MAX;
	TArray<FWFCCoordinate> CenterCandidates;

	for (const auto& [Coord, Cell] : Grid)
	{
		if (Cell.IsCollapsed() || Cell.GetPossibleTileCount() == 0) continue;

		float Distance = FVector::Dist(
			FVector(Coord.X, Coord.Y, Coord.Z),
			FVector(Center.X, Center.Y, Center.Z)
		);

		if (Distance < MinDistance ||
			(FMath::IsNearlyEqual(Distance, MinDistance, 0.5f) && Cell.Entropy < MinEntropy))
		{
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				MinEntropy = Cell.Entropy;
				CenterCandidates.Empty();
				CenterCandidates.Add(Coord);
			}
			else if (FMath::IsNearlyEqual(Distance, MinDistance, 0.5f))
			{
				if (Cell.Entropy < MinEntropy)
				{
					MinEntropy = Cell.Entropy;
					CenterCandidates.Empty();
					CenterCandidates.Add(Coord);
				}
				else if (FMath::IsNearlyEqual(Cell.Entropy, MinEntropy, 0.001f))
				{
					CenterCandidates.Add(Coord);
				}
			}
		}
	}

	if (CenterCandidates.Num() > 0)
	{
		int32 SelectedIndex = RandomGenerator.RandRange(0, CenterCandidates.Num() - 1);
		return CenterCandidates[SelectedIndex];
	}

	return FWFCCoordinate(-1, -1, -1);
}

bool FWFCCore::CollapseCell(const FWFCCoordinate& Coord)
{
	FWFCCell* Cell = GetCell(Coord);
	if (!Cell || Cell->IsCollapsed())
	{
		return false;
	}

	if (Cell->GetPossibleTileCount() == 0)
	{
		return false;
	}

	int32 SelectedTile = SelectRandomTile(*Cell, Coord);

	if (SelectedTile < 0)
	{
		return false;
	}


	if (!CheckConstraints(Coord, SelectedTile))
	{
		return false;
	}

	Cell->bCollapsed = true;
	Cell->CollapsedTileIndex = SelectedTile;
	Cell->PossibleTiles.SetRange(0, Cell->PossibleTiles.Num(), false);
	Cell->PossibleTiles[SelectedTile] = true;
	Cell->Entropy = 0.0f;

	TileInstanceCounts.FindOrAdd(SelectedTile, 0)++;

	CollapseHistory.Add(Coord);

	QueuePropagation(Coord);
	
	if (OnStatusUpdate.IsBound())
	{
		AsyncTask(ENamedThreads::GameThread, [this, Coord, SelectedTile]()
		{
			OnStatusUpdate.Execute(Coord, SelectedTile);
		});
	}

	return true;
}

bool FWFCCore::CollapseCellTo(const FWFCCoordinate& Coord, int32 TileIndex)
{
	FWFCCell* Cell = GetCell(Coord);
	if (!Cell || Cell->IsCollapsed())
	{
		return false;
	}

	if (Cell->GetPossibleTileCount() == 0)
	{
		return false;
	}

	int32 SelectedTile = 0;

	Cell->bCollapsed = true;
	Cell->CollapsedTileIndex = SelectedTile;
	Cell->PossibleTiles.SetRange(0, Cell->PossibleTiles.Num(), false);
	Cell->PossibleTiles[SelectedTile] = true;
	Cell->Entropy = 0.0f;

	TileInstanceCounts.FindOrAdd(SelectedTile, 0)++;

	CollapseHistory.Add(Coord);

	QueuePropagation(Coord);
	
	if (OnStatusUpdate.IsBound())
	{
		AsyncTask(ENamedThreads::GameThread, [this, Coord, SelectedTile]()
		{
			OnStatusUpdate.Execute(Coord, SelectedTile);
		});
	}

	return true;
}

int32 FWFCCore::SelectRandomTile(const FWFCCell& Cell, const FWFCCoordinate& Coord)
{
	TArray<int32> ValidTiles;
	TArray<float> Weights;

	for (int32 i = 0; i < Cell.PossibleTiles.Num(); i++)
	{
		if (Cell.PossibleTiles[i])
		{
			FWFCTileDefinition TileDef = TileSet->GetTile(i);
			if (!CheckDecorators(i, TileDef, Coord))
			{
				continue;
			}

			ValidTiles.Add(i);
			Weights.Add(FMath::Max(TileDef.Weight, 0.01f)); //确保权重为正
		}
	}

	if (ValidTiles.Num() == 0)
	{
		return -1;
	}

	if (ValidTiles.Num() == 1)
	{
		return ValidTiles[0];
	}

	float TotalWeight = 0.0f;
	for (float Weight : Weights)
	{
		TotalWeight += Weight;
	}

	if (TotalWeight <= 0.0f)
	{
		return ValidTiles[RandomGenerator.RandRange(0, ValidTiles.Num() - 1)];
	}

	float RandomValue = RandomGenerator.FRandRange(0.0f, TotalWeight);
	float CurrentWeight = 0.0f;

	for (int32 i = 0; i < ValidTiles.Num(); i++)
	{
		//跳过empty连接方块，将其作为最后保底选择
		if (TileSet->GetTile(ValidTiles[i]).Category == EWFCTileCategory::Empty)
		{
			continue;
		}
		CurrentWeight += Weights[i];
		if (RandomValue <= CurrentWeight)
		{
			return ValidTiles[i];
		}
	}

	return ValidTiles.Last();
}

bool FWFCCore::PropagateConstraints()
{
	int32 PropagationSteps = 0;
	const int32 MaxPropagationSteps = Config.GridSize.X * Config.GridSize.Y * Config.GridSize.Z * 10;

	while (!PropagationQueue.IsEmpty() && PropagationSteps < MaxPropagationSteps)
	{
		
		FWFCCoordinate CurrentCoord;
		PropagationQueue.Dequeue(CurrentCoord);
		PropagationSteps++;
		if (!PropagateFrom(CurrentCoord))
		{
			return false;
		}
	}

	if (PropagationSteps >= MaxPropagationSteps)
	{
		return false;
	}

	return true;
}

bool FWFCCore::PropagateFrom(const FWFCCoordinate& Coord)
{
	const FWFCCell* SourceCell = GetCell(Coord);
	if (!SourceCell)
	{
		return true;
	}

	for (int32 Dir = 0; Dir < 6; Dir++)
	{
		FWFCCoordinate NeighborCoord = GetNeighbor(Coord, static_cast<EWFCDirection>(Dir));
		FWFCCell* NeighborCell = GetCell(NeighborCoord);

		if (!NeighborCell || NeighborCell->IsCollapsed())
		{
			continue;
		}

		TArray<int32> TilesToRemove;
		for (int32 NeighborTile = 0; NeighborTile < NeighborCell->PossibleTiles.Num(); NeighborTile++)
		{
			if (!NeighborCell->PossibleTiles[NeighborTile])
			{
				continue;
			}

			bool HasSupport = false;
			for (int32 SourceTile = 0; SourceTile < SourceCell->PossibleTiles.Num(); SourceTile++)
			{
				if (SourceCell->PossibleTiles[SourceTile] &&
					PropagationRules[Dir][SourceTile].Contains(NeighborTile))
				{
					HasSupport = true;
					break;
				}
			}

			if (!HasSupport)
			{
				TilesToRemove.Add(NeighborTile);
			}
		}

		for (int32 TileToRemove : TilesToRemove)
		{
			if (!RemoveTileOption(NeighborCoord, TileToRemove))
			{
				return false;
			}

		}
	}

	return true;
}

bool FWFCCore::RemoveTileOption(const FWFCCoordinate& Coord, int32 TileIndex, bool bTrackChanges)
{
	auto tile = TileSet->GetTile(TileIndex);
	FWFCCell* Cell = GetCell(Coord);

	if (!Cell || TileIndex < 0 || TileIndex >= Cell->PossibleTiles.Num() || !Cell->PossibleTiles[TileIndex])
	{
		return true;
	}

	Cell->PossibleTiles[TileIndex] = false;
	Cell->Entropy = CalculateEntropy(*Cell);

	int32 RemainingOptions = Cell->GetPossibleTileCount();
	if (RemainingOptions == 0)
	{
		return false;
	}

	if (RemainingOptions == 1 && !Cell->IsCollapsed())
	{
		for (int32 i = 0; i < Cell->PossibleTiles.Num(); i++)
		{
			if (Cell->PossibleTiles[i])
			{
				Cell->bCollapsed = true;
				Cell->CollapsedTileIndex = i;
				TileInstanceCounts.FindOrAdd(i, 0)++;
				CollapseHistory.Add(Coord);

				if (OnStatusUpdate.IsBound())
				{
					AsyncTask(ENamedThreads::GameThread, [this, Coord, i]()
					{
						OnStatusUpdate.Execute(Coord, i);
					});
				}
				UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Auto-collapsed cell %s to tile %d"),
				       *Coord.ToString(), i);
				break;
			}
		}
	}

	QueuePropagation(Coord);

	return true;
}

void FWFCCore::QueuePropagation(const FWFCCoordinate& Coord)
{
	PropagationQueue.Enqueue(Coord);
}

float FWFCCore::CalculateEntropy(const FWFCCell& Cell) const
{
	if (Cell.IsCollapsed())
	{
		return 0.0f;
	}

	float TotalWeight = 0.0f;
	float WeightLogWeight = 0.0f;

	for (int32 i = 0; i < Cell.PossibleTiles.Num(); i++)
	{
		if (Cell.PossibleTiles[i])
		{
			float Weight = TileSet->GetTile(i).Weight;
			TotalWeight += Weight;
			if (Weight > 0.0f)
			{
				WeightLogWeight += Weight * FMath::Loge(Weight);
			}
		}
	}

	if (TotalWeight > 0.0f)
	{
		return FMath::Loge(TotalWeight) - (WeightLogWeight / TotalWeight);
	}

	return 0.0f;
}

//TODO:InstanceLimit的检查有问题，需要修改
bool FWFCCore::CheckConstraints(const FWFCCoordinate& Coord, int32 TileIndex) const
{
	/*if (!CheckInstanceLimits(TileIndex))
	{
		return false;
	}*/

	if (!CheckSupportRequirement(Coord, TileIndex))
	{
		return false;
	}

	return true;
}

bool FWFCCore::CheckInstanceLimits(int32 TileIndex) const
{
	FWFCTileDefinition TileDef = TileSet->GetTile(TileIndex);
	if (TileDef.TileName.Equals("smock_stack_bottom"))
	{
		FString TileName = TileDef.TileName;
		if (TileDef.MaxInstancesPerGeneration <= 0)
		{
			return true;
		}
	}


	int32 CurrentCount = TileInstanceCounts.FindRef(TileIndex);
	return CurrentCount < TileDef.MaxInstancesPerGeneration;
}

bool FWFCCore::CheckSupportRequirement(const FWFCCoordinate& Coord, int32 TileIndex) const
{
	FWFCTileDefinition TileDef = TileSet->GetTile(TileIndex);
	if (!TileDef.bRequiresSupport)
	{
		return true;
	}

	FWFCCoordinate BelowCoord = GetNeighbor(Coord, EWFCDirection::Down);
	const FWFCCell* BelowCell = GetCell(BelowCoord);

	if (!BelowCell)
	{
		return Coord.Z == 0;
	}

	if (BelowCell->IsCollapsed())
	{
		FWFCTileDefinition BelowTile = TileSet->GetTile(BelowCell->CollapsedTileIndex);
		return BelowTile.Category != EWFCTileCategory::Empty;
	}

	for (int32 i = 0; i < BelowCell->PossibleTiles.Num(); i++)
	{
		if (BelowCell->PossibleTiles[i])
		{
			FWFCTileDefinition BelowTile = TileSet->GetTile(i);
			if (BelowTile.Category != EWFCTileCategory::Empty)
			{
				return true;
			}
		}
	}

	return false;
}

bool FWFCCore::IsValidCoordinate(const FWFCCoordinate& Coord) const
{
	return Coord.X >= 0 && Coord.X < Config.GridSize.X &&
		Coord.Y >= 0 && Coord.Y < Config.GridSize.Y &&
		Coord.Z >= 0 && Coord.Z < Config.GridSize.Z;
}

bool FWFCCore::IsValidCoordinate(int X, int Y, int Z) const
{
	return X >= 0 && X < Config.GridSize.X &&
		Y >= 0 && Y < Config.GridSize.Y &&
		Z >= 0 && Z < Config.GridSize.Z;
}

bool FWFCCore::IsEdgeCoordinate(const FWFCCoordinate& Coord) const
{
	return Coord.X == 0 || Coord.X == Config.GridSize.X - 1 ||
		Coord.Y == 0 || Coord.Y == Config.GridSize.Y - 1 ||
		Coord.Z == 0 || Coord.Z == Config.GridSize.Z - 1;
}

bool FWFCCore::IsBoundaryCoordinate(const FWFCCoordinate& Coord) const
{
	return Coord.X == 0 || Coord.X == Config.GridSize.X - 1 ||
		Coord.Y == 0 || Coord.Y == Config.GridSize.Y - 1 ||
		Coord.Z == 0 || Coord.Z == Config.GridSize.Z - 1;
}

//因为在preprocess的时候，将整个grid四周填上了empty方块，因此地面从1开始算
bool FWFCCore::IsGroundCoordinate(const FWFCCoordinate& Coord) const
{
	return Coord.Z == 1;
}

bool FWFCCore::CheckDecorators(int TileIndex, const FWFCTileDefinition& Tile, const FWFCCoordinate& Coord) const
{
	if (!IsGroundCoordinate(Coord) && Tile.Category == EWFCTileCategory::Ground)
	{
		return false;
	}
	if (TileInstanceCounts.Contains(TileIndex) && Tile.MaxInstancesPerGeneration > 0)
	{
		if (TileInstanceCounts[TileIndex] >= Tile.MaxInstancesPerGeneration)
		{
			return false;
		}
	}

	return true;
}

bool FWFCCore::CheckCanAtEdge(const FWFCTileDefinition& Tile, const FWFCCoordinate& Coord) const
{
	for (int d = 0; d < 6; d++)
	{
		int X = Coord.X + DirectionVectors[d].X;
		int Y = Coord.Y + DirectionVectors[d].Y;
		int Z = Coord.Z + DirectionVectors[d].Z;
		FWFCSocket CurSocket = TileSet->GetSocketDefinition(Tile.Sockets[d]);
		if (!IsValidCoordinate(X, Y, Z) && !CurSocket.bAllowEmpty)
		{
			return false;
		}
	}
	return true;
}


FWFCCoordinate FWFCCore::GetNeighbor(const FWFCCoordinate& Coord, EWFCDirection Direction) const
{
	FIntVector Offset = DirectionVectors[static_cast<int32>(Direction)];
	FWFCCoordinate Neighbor(Coord.X + Offset.X, Coord.Y + Offset.Y, Coord.Z + Offset.Z);

	// 处理周期性边界
	if (Config.bPeriodicBoundary)
	{
		Neighbor.X = (Neighbor.X + Config.GridSize.X) % Config.GridSize.X;
		Neighbor.Y = (Neighbor.Y + Config.GridSize.Y) % Config.GridSize.Y;
		Neighbor.Z = (Neighbor.Z + Config.GridSize.Z) % Config.GridSize.Z;
		return Neighbor;
	}

	if (!IsValidCoordinate(Neighbor))
	{
		return FWFCCoordinate(-1, -1, -1);
	}

	return Neighbor;
}

FWFCCell* FWFCCore::GetCell(const FWFCCoordinate& Coord)
{
	return Grid.Find(Coord);
}

const FWFCCell* FWFCCore::GetCell(const FWFCCoordinate& Coord) const
{
	return Grid.Find(Coord);
}

void FWFCCore::Reset()
{
	Grid.Empty();
	CollapseHistory.Empty();
	TileInstanceCounts.Empty();
	PositionConstraints.Empty();
	PropagationRules.Empty();

	while (!PropagationQueue.IsEmpty())
	{
		FWFCCoordinate Dummy;
		PropagationQueue.Dequeue(Dummy);
	}
}

TArray<FWFCCoordinate> FWFCCore::GetNeighbors(const FWFCCoordinate& Coord) const
{
	TArray<FWFCCoordinate> Neighbors;

	for (int32 Dir = 0; Dir < 6; Dir++)
	{
		FWFCCoordinate Neighbor = GetNeighbor(Coord, static_cast<EWFCDirection>(Dir));
		if (IsValidCoordinate(Neighbor))
		{
			Neighbors.Add(Neighbor);
		}
	}

	return Neighbors;
}

void FWFCCore::SetPreProcessCache(UWFCPreProcessCache* InCache)
{
	PreProcessCache = InCache;
}

bool FWFCCore::LoadPreProcessedGrid()
{
	if (!PreProcessCache)
	{
		return false;
	}

	FWFCPreProcessCacheData CacheData;
	if (PreProcessCache->GetCacheForGridSize(Config.GridSize, CacheData))
	{
		ApplyCachedGrid(CacheData);
		return true;
	}

	return false;
}

void FWFCCore::ApplyCachedGrid(const FWFCPreProcessCacheData& CacheData)
{
	Grid.Empty();
	TileInstanceCounts = CacheData.CachedTileInstanceCounts;
	CollapseHistory = CacheData.CachedCollapseHistory;

	for (const auto& [Coord, CachedCell] : CacheData.CachedGrid)
	{
		FWFCCell& Cell = Grid.Add(Coord, FWFCCell(TileSet->GetTileCount()));
        
		Cell.PossibleTiles.SetNum(CachedCell.PossibleTiles.Num(), false);
		for (int32 i = 0; i < CachedCell.PossibleTiles.Num(); i++)
		{
			Cell.PossibleTiles[i] = CachedCell.PossibleTiles[i];
		}
        
		Cell.bCollapsed = CachedCell.bCollapsed;
		Cell.CollapsedTileIndex = CachedCell.CollapsedTileIndex;
		Cell.Entropy = CachedCell.Entropy;
	}

	while (!PropagationQueue.IsEmpty())
	{
		FWFCCoordinate Dummy;
		PropagationQueue.Dequeue(Dummy);
	}
}