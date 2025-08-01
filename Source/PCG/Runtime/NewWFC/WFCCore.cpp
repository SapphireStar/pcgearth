// Fill out your copyright notice in the Description page of Project Settings.

#include "WFCCore.h"

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
		UE_LOG(LogTemp, Error, TEXT("WFCCore: TileSet is null"));
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

	UE_LOG(LogTemp, Log, TEXT("WFCCore: Initializing with %d tiles, grid size %s, seed %d"),
	       TileSet->GetTileCount(), *Config.GridSize.ToString(), Config.RandomSeed);

	Reset();

	InitializeGrid();
	BuildPropagationRules();
	//ApplyConstraints();

	UE_LOG(LogTemp, Log, TEXT("WFCCore: Initialization complete"));
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
		UE_LOG(LogTemp, Error, TEXT("WFCCore: TileSet is null"));
		return;
	}

	const int32 TileCount = TileSet->GetTileCount();
	const int32 TotalCells = Config.GridSize.X * Config.GridSize.Y * Config.GridSize.Z;

	UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Initializing grid with %d cells, %d tile types"),
	       TotalCells, TileCount);

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

				UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Initialized cell %s with entropy %.3f"),
				       *Coord.ToString(), Cell.Entropy);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("WFCCore: Grid initialization complete - %d cells created"), Grid.Num());
}

void FWFCCore::BuildPropagationRules()
{
	const int32 TileCount = TileSet->GetTileCount();
	PropagationRules.Empty();
	PropagationRules.SetNum(6);

	UE_LOG(LogTemp, Log, TEXT("WFCCore: Building propagation rules for %d tiles"), TileCount);

	for (int32 Dir = 0; Dir < 6; Dir++)
	{
		PropagationRules[Dir].SetNum(TileCount);
		EWFCDirection Direction = static_cast<EWFCDirection>(Dir);
		EWFCDirection OppositeDirection = static_cast<EWFCDirection>(Dir ^ 1);

		const TCHAR* DirectionNames[] = {
			TEXT("Up"), TEXT("Down"), TEXT("North"), TEXT("South"), TEXT("East"), TEXT("West")
		};

		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Processing direction %s (opposite: %s)"),
		       DirectionNames[Dir], DirectionNames[Dir ^ 1]);

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

					UE_LOG(LogTemp, VeryVerbose,
					       TEXT("WFCCore: Rule %s: Tile %d (%s) -> Tile %d (%s) via sockets '%s' <-> '%s'"),
					       DirectionNames[Dir], TileA, *TileDefA.TileName, TileB, *TileDefB.TileName, *SocketA,
					       *SocketB);
				}
			}

			UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Tile %d has %d compatible neighbors in direction %s"),
			       TileA, PropagationRules[Dir][TileA].Num(), DirectionNames[Dir]);
		}

		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Direction %s has %d total compatibility rules"),
		       DirectionNames[Dir], TotalRules);
	}

	UE_LOG(LogTemp, Log, TEXT("WFCCore: Propagation rules building complete"));

	ValidatePropagationRules();
}

void FWFCCore::ValidatePropagationRules()
{
	UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Validating propagation rules symmetry"));

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
					UE_LOG(LogTemp, Warning,
					       TEXT("WFCCore: Asymmetric rule found - Tile %d -> %d in dir %d, but not reverse"),
					       TileA, TileB, Dir);
					bFoundAsymmetry = true;
				}
			}
		}
	}

	if (!bFoundAsymmetry)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Propagation rules symmetry validation passed"));
	}
}

void FWFCCore::ApplyConstraints()
{
	PositionConstraints.Empty();

	UE_LOG(LogTemp, Log, TEXT("WFCCore: Applying %d constraints"), Config.Constraints.Num());

	for (const FWFCGenerationConstraint& Constraint : Config.Constraints)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Applying constraint: %s"), *Constraint.ConstraintName);

		for (const FWFCCoordinate& Pos : Constraint.RequiredPositions)
		{
			if (IsValidCoordinate(Pos))
			{
				PositionConstraints.FindOrAdd(Pos) = Constraint.AllowedTileIndices;
				UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Position %s constrained to %d allowed tiles"),
				       *Pos.ToString(), Constraint.AllowedTileIndices.Num());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("WFCCore: Invalid required position in constraint: %s"), *Pos.ToString());
			}
		}

		for (const FWFCCoordinate& Pos : Constraint.ForbiddenPositions)
		{
			if (FWFCCell* Cell = GetCell(Pos))
			{
				for (int32 ForbiddenTile : Constraint.ForbiddenTileIndices)
				{
					if (ForbiddenTile >= 0 && ForbiddenTile < Cell->PossibleTiles.Num())
					{
						RemoveTileOption(Pos, ForbiddenTile, false);
						UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Removed tile %d from position %s (forbidden)"),
						       ForbiddenTile, *Pos.ToString());
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("WFCCore: Invalid forbidden position in constraint: %s"),
				       *Pos.ToString());
			}
		}

		if (Constraint.MinLayer >= 0 || Constraint.MaxLayer >= 0)
		{
			int32 AffectedCells = 0;
			for (auto& [Coord, Cell] : Grid)
			{
				bool bInRange = true;
				if (Constraint.MinLayer >= 0 && Coord.Z < Constraint.MinLayer)
				{
					bInRange = false;
				}
				if (Constraint.MaxLayer >= 0 && Coord.Z > Constraint.MaxLayer)
				{
					bInRange = false;
				}

				if (!bInRange)
				{
					for (int32 ForbiddenTile : Constraint.ForbiddenTileIndices)
					{
						if (ForbiddenTile >= 0 && ForbiddenTile < Cell.PossibleTiles.Num())
						{
							RemoveTileOption(Coord, ForbiddenTile, false);
							AffectedCells++;
						}
					}
				}
			}

			UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Layer constraint affected %d cells"), AffectedCells);
		}
	}

	if (!PropagationQueue.IsEmpty())
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Running initial constraint propagation"));
		PropagateConstraints();
	}

	UE_LOG(LogTemp, Log, TEXT("WFCCore: Constraint application complete"));
}

void FWFCCore::CellPreProcess()
{
	FRandomStream Stream;
	int rand = Stream.FRandRange(0,10);
	
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

	UE_LOG(LogTemp, Log, TEXT("WFCCore: Starting generation with mode %d, max iterations %d"),
	       (int32)Config.GenerationMode, Config.MaxIterations);

	TileInstanceCounts.Empty();
	ChangeHistory.Empty();
	CollapseHistory.Empty();
	InitializeGrid();

	while (!PropagationQueue.IsEmpty())
	{
		FWFCCoordinate Dummy;
		PropagationQueue.Dequeue(Dummy);
	}

	CellPreProcess();

	Result.bSuccess = RunGenerationLoop();

	if (Result.bSuccess)
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

		UE_LOG(LogTemp, Log, TEXT("WFCCore: Generation succeeded with %d placed tiles, %d failed positions"),
		       Result.TileAssignments.Num(), Result.FailedPositions.Num());
	}
	else
	{
		Result.ErrorMessage = TEXT("Generation failed - contradiction detected or max iterations reached");
		UE_LOG(LogTemp, Warning, TEXT("WFCCore: %s"), *Result.ErrorMessage);

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

	UE_LOG(LogTemp, Log, TEXT("WFCCore: Generation completed in %.3f seconds with %d iterations"),
	       Result.GenerationTimeSeconds, Result.IterationsUsed);

	return Result;
}

bool FWFCCore::RunGenerationLoop()
{
	UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Starting generation loop"));

	for (int32 Iteration = 0; Iteration < Config.MaxIterations; Iteration++)
	{
		FWFCCoordinate NextCoord = SelectNextCell();

		if (NextCoord == FWFCCoordinate(-1, -1, -1))
		{
			UE_LOG(LogTemp, Log, TEXT("WFCCore: Generation completed at iteration %d - no more cells to collapse"),
			       Iteration);
			return true;
		}

		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Iteration %d - selected cell %s"), Iteration,
		       *NextCoord.ToString());

		if (Config.bEnableBacktracking)
		{
			SaveState();
		}

		// 坍缩选中的单元
		if (!CollapseCell(NextCoord))
		{
			UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Collapse failed for cell %s"), *NextCoord.ToString());

			if (Config.bEnableBacktracking && CanBacktrack())
			{
				UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Attempting backtrack"));
				if (Backtrack())
				{
					continue;
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("WFCCore: Generation failed at iteration %d - collapse failed"), Iteration);
			return false;
		}

		if (!PropagateConstraints())
		{
			UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Propagation failed after collapsing cell %s"),
			       *NextCoord.ToString());

			if (Config.bEnableBacktracking && CanBacktrack())
			{
				UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Attempting backtrack after propagation failure"));
				if (Backtrack())
				{
					continue;
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("WFCCore: Generation failed at iteration %d - propagation failed"),
			       Iteration);
			return false;
		}

		if (Iteration % 100 == 0)
		{
			UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Progress - Iteration %d, %s"), Iteration,
			       *GetGridStateString());
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("WFCCore: Reached maximum iterations (%d) without completion"), Config.MaxIterations);
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
		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Random selection - %d candidates with entropy %.3f, selected %s"),
		       Candidates.Num(), MinEntropy, *Candidates[SelectedIndex].ToString());
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
		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: GroundFirst selection - %d ground candidates, selected %s"),
		       GroundCandidates.Num(), *GroundCandidates[SelectedIndex].ToString());
		return GroundCandidates[SelectedIndex];
	}

	if (OtherCandidates.Num() > 0)
	{
		int32 SelectedIndex = RandomGenerator.RandRange(0, OtherCandidates.Num() - 1);
		UE_LOG(LogTemp, VeryVerbose,
		       TEXT("WFCCore: GroundFirst selection - no ground candidates, %d other candidates, selected %s"),
		       OtherCandidates.Num(), *OtherCandidates[SelectedIndex].ToString());
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

		for (int32 X = 0; X < Config.GridSize.X; X++)
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
			UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Layered selection - layer %d, %d candidates, selected %s"),
			       Z, LayerCandidates.Num(), *LayerCandidates[SelectedIndex].ToString());
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
		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: CenterOut selection - distance %.1f, %d candidates, selected %s"),
		       MinDistance, CenterCandidates.Num(), *CenterCandidates[SelectedIndex].ToString());
		return CenterCandidates[SelectedIndex];
	}

	return FWFCCoordinate(-1, -1, -1);
}

bool FWFCCore::CollapseCell(const FWFCCoordinate& Coord)
{
	FWFCCell* Cell = GetCell(Coord);
	if (!Cell || Cell->IsCollapsed())
	{
		UE_LOG(LogTemp, Warning, TEXT("WFCCore: Cannot collapse - cell is null or already collapsed at %s"),
		       *Coord.ToString());
		return false;
	}

	if (Cell->GetPossibleTileCount() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("WFCCore: Cannot collapse - no possible tiles at %s"), *Coord.ToString());
		return false;
	}

	int32 SelectedTile = SelectRandomTile(*Cell, Coord);

	if (SelectedTile < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("WFCCore: Failed to select tile for collapse at %s"), *Coord.ToString());
		return false;
	}

	if (!CheckConstraints(Coord, SelectedTile))
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Constraint check failed for tile %d at %s"),
		       SelectedTile, *Coord.ToString());
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

	LogGenerationStep(Coord, SelectedTile);

	return true;
}

bool FWFCCore::CollapseCellTo(const FWFCCoordinate& Coord, int32 TileIndex)
{
	FWFCCell* Cell = GetCell(Coord);
	if (!Cell || Cell->IsCollapsed())
	{
		UE_LOG(LogTemp, Warning, TEXT("WFCCore: Cannot collapse - cell is null or already collapsed at %s"),
		       *Coord.ToString());
		return false;
	}

	if (Cell->GetPossibleTileCount() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("WFCCore: Cannot collapse - no possible tiles at %s"), *Coord.ToString());
		return false;
	}


	FWFCTileDefinition TileDef = TileSet->GetTile(TileIndex);
	if (!CheckDecorators(TileIndex, TileDef, Coord))
	{
		return false;
	}
	/*if (IsEdgeCoordinate(Coord) && !CheckCanAtEdge(TileDef, Coord))
	{
		UE_LOG(LogTemp, Warning, TEXT("WFCCore: Try collapse %s to tile %s failed"), *Coord.ToString(), *TileSet->GetTile(TileIndex).TileName);
		return false;
	}*/

	if (TileIndex < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("WFCCore: Failed to select tile for collapse at %s"), *Coord.ToString());
		return false;
	}

	if (!CheckConstraints(Coord, TileIndex))
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Constraint check failed for tile %d at %s"),
		       TileIndex, *Coord.ToString());
		return false;
	}

	Cell->bCollapsed = true;
	Cell->CollapsedTileIndex = TileIndex;
	Cell->PossibleTiles.SetRange(0, Cell->PossibleTiles.Num(), false);
	Cell->PossibleTiles[TileIndex] = true;
	Cell->Entropy = 0.0f;

	TileInstanceCounts.FindOrAdd(TileIndex, 0)++;

	CollapseHistory.Add(Coord);

	QueuePropagation(Coord);

	LogGenerationStep(Coord, TileIndex);

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

			/*if (IsEdgeCoordinate(Coord) && !CheckCanAtEdge(TileDef, Coord))
			{
				continue;
			}*/
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
			UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Propagation failed from %s at step %d"),
			       *CurrentCoord.ToString(), PropagationSteps);
			return false;
		}
	}

	if (PropagationSteps >= MaxPropagationSteps)
	{
		UE_LOG(LogTemp, Warning, TEXT("WFCCore: Propagation reached maximum steps (%d), possible infinite loop"),
		       MaxPropagationSteps);
		return false;
	}

	UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Propagation completed in %d steps"), PropagationSteps);
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

			LogPropagationStep(Coord, NeighborCoord, TileToRemove);
		}
	}

	return true;
}

bool FWFCCore::RemoveTileOption(const FWFCCoordinate& Coord, int32 TileIndex, bool bTrackChanges)
{
	FWFCCell* Cell = GetCell(Coord);
	if (!Cell || TileIndex < 0 || TileIndex >= Cell->PossibleTiles.Num() || !Cell->PossibleTiles[TileIndex])
	{
		return true;
	}

	if (bTrackChanges && ChangeHistory.Num() > 0)
	{
		ChangeHistory.Last().Emplace(Coord, TileIndex, true);
	}

	Cell->PossibleTiles[TileIndex] = false;
	Cell->Entropy = CalculateEntropy(*Cell);

	int32 RemainingOptions = Cell->GetPossibleTileCount();
	if (RemainingOptions == 0)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Cell at %s has no remaining options after removing tile %d"),
		       *Coord.ToString(), TileIndex);
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

bool FWFCCore::CanBacktrack() const
{
	return Config.bEnableBacktracking &&
		ChangeHistory.Num() > 0 &&
		ChangeHistory.Num() <= Config.BacktrackingDepth;
}

void FWFCCore::SaveState()
{
	ChangeHistory.Emplace();
}

bool FWFCCore::Backtrack()
{
	if (ChangeHistory.Num() == 0)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Cannot backtrack - no history available"));
		return false;
	}

	UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Backtracking from depth %d"), ChangeHistory.Num());

	const TArray<FWFCChange>& LastChanges = ChangeHistory.Last();
	for (int32 i = LastChanges.Num() - 1; i >= 0; i--)
	{
		const FWFCChange& Change = LastChanges[i];
		FWFCCell* Cell = GetCell(Change.Position);
		if (Cell && Change.TileIndex >= 0 && Change.TileIndex < Cell->PossibleTiles.Num())
		{
			Cell->PossibleTiles[Change.TileIndex] = !Change.bWasRemoved;
			Cell->Entropy = CalculateEntropy(*Cell);

			UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Restored tile %d at %s (was %s)"),
			       Change.TileIndex, *Change.Position.ToString(), Change.bWasRemoved ? TEXT("removed") : TEXT("added"));
		}
	}

	ChangeHistory.Pop();
	if (CollapseHistory.Num() > 0)
	{
		FWFCCoordinate LastCollapse = CollapseHistory.Pop();
		FWFCCell* Cell = GetCell(LastCollapse);
		if (Cell)
		{
			int32 CollapsedTile = Cell->CollapsedTileIndex;
			Cell->bCollapsed = false;
			Cell->CollapsedTileIndex = -1;

			if (int32* Count = TileInstanceCounts.Find(CollapsedTile))
			{
				(*Count)--;
				if (*Count <= 0)
				{
					TileInstanceCounts.Remove(CollapsedTile);
				}
			}

			Cell->Entropy = CalculateEntropy(*Cell);

			UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Uncollapsed cell %s (was tile %d)"),
			       *LastCollapse.ToString(), CollapsedTile);
		}
	}

	return true;
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
	if (TileInstanceCounts.Contains(TileIndex) && Tile.MaxInstancesPerGeneration>0)
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
	UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Resetting state"));

	Grid.Empty();
	ChangeHistory.Empty();
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

void FWFCCore::LogGenerationStep(const FWFCCoordinate& Coord, int32 TileIndex) const
{
	if (TileSet)
	{
		FWFCTileDefinition TileDef = TileSet->GetTile(TileIndex);
		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Collapsed %s to tile %d (%s)"),
		       *Coord.ToString(), TileIndex, *TileDef.TileName);
	}
	else
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Collapsed %s to tile %d"),
		       *Coord.ToString(), TileIndex);
	}
}

void FWFCCore::LogPropagationStep(const FWFCCoordinate& From, const FWFCCoordinate& To, int32 RemovedTile) const
{
	UE_LOG(LogTemp, VeryVerbose, TEXT("WFCCore: Propagation %s -> %s removed tile %d"),
	       *From.ToString(), *To.ToString(), RemovedTile);
}

FString FWFCCore::GetGridStateString() const
{
	FString StateString;
	int32 CollapsedCount = 0;
	int32 TotalCells = Grid.Num();

	for (const auto& [Coord, Cell] : Grid)
	{
		if (Cell.IsCollapsed())
		{
			CollapsedCount++;
		}
	}

	StateString = FString::Printf(TEXT("Grid State: %d/%d collapsed (%.1f%%)"),
	                              CollapsedCount, TotalCells,
	                              TotalCells > 0 ? (float)CollapsedCount / TotalCells * 100.0f : 0.0f);

	return StateString;
}
