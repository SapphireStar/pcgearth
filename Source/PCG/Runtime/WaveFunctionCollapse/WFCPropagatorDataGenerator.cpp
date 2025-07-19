// WFCPropagatorDataGenerator.cpp - Improved version with better validation

#include "WFCPropagatorDataGenerator.h"
#include "WFCTileData.h"
#include "WFCSocketCompatibilityData.h"
#include "WFCSolver.h"

UWFCPropagatorDataGenerator::UWFCPropagatorDataGenerator(TObjectPtr<UWFCSocketCompatibilityData> socketData,
	TObjectPtr<UWFCTileData> baseTileData)
{
	this->SocketData = socketData;
	this->BaseTileData = baseTileData;
}

bool UWFCPropagatorDataGenerator::GeneratePropagatorRules(TArray<TArray<TArray<int>>>& outPropagator)
{
	if (!SocketData || !BaseTileData)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCPropagatorDataGenerator: Missing socket data or tile data"));
		return false;
	}
	
	if (BaseTileData->Tiles.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCPropagatorDataGenerator: No tiles in base tile data"));
		return false;
	}
	
	GenerateCompleteData();
	
	if (!CompleteTileData || CompleteTileData->Tiles.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCPropagatorDataGenerator: Failed to generate complete tile data"));
		return false;
	}
	
	BuildPropagatorTable();
	ValidatePropagatorRules();
	
	// 复制传播器数据
	outPropagator.Empty();
	outPropagator.SetNum(6);
	for (int d = 0; d < 6; d++)
	{
		outPropagator[d].SetNum(Propagator[d].Num());
		for (int t = 0; t < Propagator[d].Num(); t++)
		{
			outPropagator[d][t].Empty();
			for (int otherT = 0; otherT < Propagator[d][t].Num(); otherT++)
			{
				outPropagator[d][t].Add(Propagator[d][t][otherT]);
			}
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("WFCPropagatorDataGenerator: Successfully generated propagator rules for %d tiles"), 
		CompleteTileData->Tiles.Num());
	
	return true;
}

int UWFCPropagatorDataGenerator::GetTileCount() const
{
	return CompleteTileData ? CompleteTileData->Tiles.Num() : 0;
}

const FWFCTile UWFCPropagatorDataGenerator::GetTile(int TileID) const
{
	if (CompleteTileData && TileID >= 0 && TileID < CompleteTileData->Tiles.Num())
	{
		return CompleteTileData->Tiles[TileID];
	}
	
	UE_LOG(LogTemp, Warning, TEXT("WFCPropagatorDataGenerator: Invalid tile ID %d"), TileID);
	return FWFCTile();
}

UWFCTileData* UWFCPropagatorDataGenerator::GetCompleteData() const
{
	return CompleteTileData;
}

void UWFCPropagatorDataGenerator::GenerateCompleteData()
{
	CompleteTileData = NewObject<UWFCTileData>();
	
	UE_LOG(LogTemp, Log, TEXT("WFCPropagatorDataGenerator: Generating complete tile data from %d base tiles"), 
		BaseTileData->Tiles.Num());
	
	for (int i = 0; i < BaseTileData->Tiles.Num(); i++)
	{
		const FWFCTile& baseTile = BaseTileData->Tiles[i];
		
		// 验证基础tile的socket配置
		if (baseTile.Sockets.Num() != 6)
		{
			UE_LOG(LogTemp, Warning, TEXT("WFCPropagatorDataGenerator: Tile %s has incorrect socket count: %d (expected 6)"), 
				*baseTile.TileName, baseTile.Sockets.Num());
			continue;
		}
		
		// 添加原始tile
		CompleteTileData->Tiles.Add(baseTile);
		
		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCPropagatorDataGenerator: Added base tile %s with sockets: [%s, %s, %s, %s, %s, %s]"), 
			*baseTile.TileName,
			*baseTile.Sockets[0], *baseTile.Sockets[1], *baseTile.Sockets[2], 
			*baseTile.Sockets[3], *baseTile.Sockets[4], *baseTile.Sockets[5]);
		
		// 创建旋转变体
		if (ShouldCreateRotationVariants(baseTile))
		{
			for (int r = 0; r < 3; r++)
			{
				FWFCTile rotatedTile;
				TArray<FString> rotatedSockets = RotateSocketsZ90(baseTile.Sockets);
				float rotationAngle = 90.f * (r + 1);
				
				rotatedTile.InitializeTileVariant(baseTile, rotatedSockets, rotationAngle);
				CompleteTileData->Tiles.Add(rotatedTile);
				
				UE_LOG(LogTemp, VeryVerbose, TEXT("WFCPropagatorDataGenerator: Added rotated tile %s (%d°) with sockets: [%s, %s, %s, %s, %s, %s]"), 
					*rotatedTile.TileName, (int)rotationAngle,
					*rotatedSockets[0], *rotatedSockets[1], *rotatedSockets[2], 
					*rotatedSockets[3], *rotatedSockets[4], *rotatedSockets[5]);
				

			}
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("WFCPropagatorDataGenerator: Generated %d total tiles (including rotations)"), 
		CompleteTileData->Tiles.Num());
}

bool UWFCPropagatorDataGenerator::ShouldCreateRotationVariants(const FWFCTile& TileInfo)
{
	return TileInfo.bShouldCreateRotationVariant;
}

TArray<FString> UWFCPropagatorDataGenerator::RotateSocketsZ90(const TArray<FString>& OriginalSockets)
{
	if (OriginalSockets.Num() != 6)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCPropagatorDataGenerator: Cannot rotate sockets - incorrect count: %d"), 
			OriginalSockets.Num());
		return OriginalSockets;
	}
	
	TArray<FString> RotatedSockets;
	RotatedSockets.SetNum(6);
	
	// Z轴90度旋转映射：
	// Up(0) -> Up(0), Down(1) -> Down(1) [垂直方向不变]
	// Left(2) -> Backward(5), Right(3) -> Forward(4)
	// Forward(4) -> Left(2), Backward(5) -> Right(3)
	RotatedSockets[0] = OriginalSockets[0]; // Up stays Up
	RotatedSockets[1] = OriginalSockets[1]; // Down stays Down
	RotatedSockets[2] = OriginalSockets[5]; // Left <- Backward
	RotatedSockets[3] = OriginalSockets[4]; // Right <- Forward
	RotatedSockets[4] = OriginalSockets[2]; // Forward <- Left
	RotatedSockets[5] = OriginalSockets[3]; // Backward <- Right
	
	return RotatedSockets;
}

void UWFCPropagatorDataGenerator::BuildPropagatorTable()
{
	if (!CompleteTileData)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCPropagatorDataGenerator::BuildPropagatorTable: Complete data not generated"));
		return;
	}
	
	if (!SocketData)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCPropagatorDataGenerator::BuildPropagatorTable: Socket data not available"));
		return;
	}
	
	int TileCount = CompleteTileData->Tiles.Num();
	UE_LOG(LogTemp, Log, TEXT("WFCPropagatorDataGenerator: Building propagator table for %d tiles"), TileCount);

	Propagator.Empty();
	Propagator.SetNum(6);

	// 方向名称用于调试
	const TCHAR* DirectionNames[] = { 
		TEXT("Up"), TEXT("Down"), TEXT("Left"), TEXT("Right"), TEXT("Forward"), TEXT("Backward") 
	};

	for (int d = 0; d < 6; d++)
	{
		Propagator[d].SetNum(TileCount);
		int oppositeDir = GetOppositeDirection(d);
		
		UE_LOG(LogTemp, VeryVerbose, TEXT("WFCPropagatorDataGenerator: Processing direction %s (opposite: %s)"), 
			DirectionNames[d], DirectionNames[oppositeDir]);
		
		for (int t = 0; t < TileCount; t++)
		{
			const FWFCTile& currentTile = CompleteTileData->Tiles[t];
			FString currentSocket = currentTile.GetSocketForDirection(d);
			
			if (currentSocket.IsEmpty() || currentSocket == TEXT("Empty"))
			{
				UE_LOG(LogTemp, VeryVerbose, TEXT("WFCPropagatorDataGenerator: Tile %d (%s) has empty socket in direction %s"), 
					t, *currentTile.TileName, DirectionNames[d]);
			}

			for (int otherT = 0; otherT < TileCount; otherT++)
			{
				const FWFCTile& otherTile = CompleteTileData->Tiles[otherT];
				FString otherSocket = otherTile.GetSocketForDirection(oppositeDir);
				
				// 检查socket兼容性
				bool compatible = SocketData->AreSocketsCompatible(currentSocket, otherSocket);
				
				if (compatible)
				{
					Propagator[d][t].Add(otherT);
					
					UE_LOG(LogTemp, VeryVerbose, TEXT("WFCPropagatorDataGenerator: Tile %d (%s, socket '%s') compatible with tile %d (%s, socket '%s') in direction %s"), 
						t, *currentTile.TileName, *currentSocket,
						otherT, *otherTile.TileName, *otherSocket,
						DirectionNames[d]);
				}
				else
				{
					UE_LOG(LogTemp, VeryVerbose, TEXT("WFCPropagatorDataGenerator: Tile %d (%s, socket '%s') NOT compatible with tile %d (%s, socket '%s') in direction %s"), 
						t, *currentTile.TileName, *currentSocket,
						otherT, *otherTile.TileName, *otherSocket,
						DirectionNames[d]);
				}
			}
			
			UE_LOG(LogTemp, VeryVerbose, TEXT("WFCPropagatorDataGenerator: Tile %d has %d compatible tiles in direction %s"), 
				t, Propagator[d][t].Num(), DirectionNames[d]);
		}
	}
}

void UWFCPropagatorDataGenerator::ValidatePropagatorRules()
{
	UE_LOG(LogTemp, Log, TEXT("WFCPropagatorDataGenerator: Validating propagator rules..."));
	
	if (Propagator.Num() != 6)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCPropagatorDataGenerator: Invalid propagator direction count: %d"), Propagator.Num());
		return;
	}
	
	int TileCount = CompleteTileData->Tiles.Num();
	bool hasErrors = false;
	
	for (int d = 0; d < 6; d++)
	{
		if (Propagator[d].Num() != TileCount)
		{
			UE_LOG(LogTemp, Error, TEXT("WFCPropagatorDataGenerator: Direction %d has incorrect tile count: %d (expected %d)"), 
				d, Propagator[d].Num(), TileCount);
			hasErrors = true;
			continue;
		}
		
		int oppositeDir = GetOppositeDirection(d);
		
		for (int t1 = 0; t1 < TileCount; t1++)
		{
			// 检查每个tile是否至少有一个兼容的neighbor
			if (Propagator[d][t1].Num() == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("WFCPropagatorDataGenerator: Tile %d (%s) has no compatible neighbors in direction %d"), 
					t1, *CompleteTileData->Tiles[t1].TileName, d);
			}
			
			// 检查对称性
			for (int t2 : Propagator[d][t1])
			{
				if (!Propagator[oppositeDir][t2].Contains(t1))
				{
					UE_LOG(LogTemp, Warning, TEXT("WFCPropagatorDataGenerator: Asymmetric rule: tile %d -> %d in direction %d, but not reverse"), 
						t1, t2, d);
					hasErrors = true;
				}
			}
		}
	}
	
	if (!hasErrors)
	{
		UE_LOG(LogTemp, Log, TEXT("WFCPropagatorDataGenerator: Propagator rules validation passed"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WFCPropagatorDataGenerator: Propagator rules validation found issues"));
	}
}

int UWFCPropagatorDataGenerator::GetOppositeDirection(int Direction)
{
	switch (Direction)
	{
	case 0: return 1; // Up -> Down
	case 1: return 0; // Down -> Up
	case 2: return 3; // Left -> Right
	case 3: return 2; // Right -> Left
	case 4: return 5; // Forward -> Backward
	case 5: return 4; // Backward -> Forward
	default:
		UE_LOG(LogTemp, Error, TEXT("WFCPropagatorDataGenerator::GetOppositeDirection: Invalid direction: %d"), Direction);
		return -1;
	}
}