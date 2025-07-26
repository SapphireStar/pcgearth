#include "WFCTileSet.h"

FWFCTileDefinition UWFCTileSet::GetTile(int32 Index) const
{
	if (Index >= 0 && Index < Tiles.Num())
	{
		return Tiles[Index];
	}

	UE_LOG(LogTemp, Warning, TEXT("WFCTileSet: Invalid tile index %d"), Index);
	return FWFCTileDefinition();
}

int32 UWFCTileSet::FindTileByName(const FString& TileName) const
{
	for (int32 i = 0; i < Tiles.Num(); i++)
	{
		if (Tiles[i].TileName.Equals(TileName, ESearchCase::IgnoreCase))
		{
			return i;
		}
	}
	return -1;
}

TArray<int32> UWFCTileSet::GetTilesByCategory(EWFCTileCategory Category) const
{
	TArray<int32> Result;
	for (int32 i = 0; i < Tiles.Num(); i++)
	{
		if (Tiles[i].Category == Category)
		{
			Result.Add(i);
		}
	}
	return Result;
}

bool UWFCTileSet::AreSocketsCompatible(const FString& Socket1, const FString& Socket2) const
{
	if (Socket1.IsEmpty() || Socket2.IsEmpty())
	{
		return Socket1.IsEmpty() && Socket2.IsEmpty();
	}
	if (Socket1.Equals("-1"))
	{
		if (!Socket2.Equals("0"))
			return false;
		else
		{
			return true;
		}
	}
	if (Socket2.Equals("-1"))
	{
		if (!Socket1.Equals("0"))
			return false;
		else
		{
			return true;
		}
	}

	if (Socket1.Contains("_") && Socket2.Contains("_"))
	{
		return Socket1.Equals(Socket2, ESearchCase::IgnoreCase);
	}

	if (Socket1.EndsWith("s") && Socket2.EndsWith("s"))
	{
		return Socket1.Equals(Socket2, ESearchCase::IgnoreCase);
	}

	if (Socket1.EndsWith("f") && !Socket2.EndsWith("f"))
	{
		FString S1 = Socket1;
		S1.RemoveFromEnd(S1.Right(1));
		return S1.Equals(Socket2, ESearchCase::IgnoreCase);
	}
	if (!Socket1.EndsWith("f") && Socket2.EndsWith("f"))
	{
		FString S2 = Socket2;
		S2.RemoveFromEnd(S2.Right(1));
		return S2.Equals(Socket1, ESearchCase::IgnoreCase);
	}

	if (Socket1.Equals("0") && Socket2.Equals("0"))
	{
		return true;
	}

	/*for (const FWFCSocket& SocketDef : SocketDefinitions)
	{
	    if (SocketDef.SocketName.Equals(Socket1, ESearchCase::IgnoreCase))
	    {
	        return SocketDef.CompatibleSockets.Contains(Socket2);
	    }
	}
	
	for (const FWFCSocket& SocketDef : SocketDefinitions)
	{
	    if (SocketDef.SocketName.Equals(Socket2, ESearchCase::IgnoreCase))
	    {
	        return SocketDef.CompatibleSockets.Contains(Socket1);
	    }
	}*/

	return false;
	//return Socket1.Equals(Socket2, ESearchCase::IgnoreCase);
}

FWFCSocket UWFCTileSet::GetSocketDefinition(const FString& SocketName) const
{
	for (const FWFCSocket& SocketDef : SocketDefinitions)
	{
		if (SocketDef.SocketName.Equals(SocketName, ESearchCase::IgnoreCase))
		{
			return SocketDef;
		}
	}

	return FWFCSocket(SocketName);
}

bool UWFCTileSet::ValidateTileSet(FString& OutErrorMessage) const
{
	TArray<FString> Errors;

	if (Tiles.Num() == 0)
	{
		Errors.Add(TEXT("No tiles defined in tile set"));
	}

	for (int32 i = 0; i < Tiles.Num(); i++)
	{
		const FWFCTileDefinition& Tile = Tiles[i];

		if (Tile.TileName.IsEmpty())
		{
			Errors.Add(FString::Printf(TEXT("Tile %d has empty name"), i));
		}

		if (Tile.Sockets.Num() != 6)
		{
			Errors.Add(FString::Printf(TEXT("Tile %d (%s) has %d sockets, expected 6"),
			                           i, *Tile.TileName, Tile.Sockets.Num()));
		}

		if (!Tile.Mesh)
		{
			Errors.Add(FString::Printf(TEXT("Tile %d (%s) has no mesh assigned"), i, *Tile.TileName));
		}
	}

	TSet<FString> DefinedSockets;
	for (const FWFCSocket& SocketDef : SocketDefinitions)
	{
		if (SocketDef.SocketName.IsEmpty())
		{
			Errors.Add(TEXT("Found socket definition with empty name"));
			continue;
		}

		if (DefinedSockets.Contains(SocketDef.SocketName))
		{
			Errors.Add(FString::Printf(TEXT("Duplicate socket definition: %s"), *SocketDef.SocketName));
		}
		DefinedSockets.Add(SocketDef.SocketName);
	}

	TSet<FString> UsedSockets;
	for (const FWFCTileDefinition& Tile : Tiles)
	{
		for (const FString& Socket : Tile.Sockets)
		{
			if (!Socket.IsEmpty())
			{
				UsedSockets.Add(Socket);
			}
		}
	}

	for (const FString& UsedSocket : UsedSockets)
	{
		if (!DefinedSockets.Contains(UsedSocket))
		{
			Errors.Add(FString::Printf(TEXT("Socket '%s' is used but not defined"), *UsedSocket));
		}
	}

	if (Errors.Num() > 0)
	{
		OutErrorMessage = FString::Join(Errors, TEXT("\n"));
		return false;
	}

	OutErrorMessage = TEXT("Tile set validation passed");
	return true;
}

void UWFCTileSet::GenerateRotationVariants()
{
	Tiles.Empty();
	SocketDefinitions.Empty();
	for (int i = 0; i < TileRuleSets.Num(); i++)
	{
		for (int j = 0; j < TileRuleSets[i].Tiles.Num(); j++)
		{
			Tiles.Add(TileRuleSets[i].Tiles[j]);
		}
	}

	for (int i = 0; i < SocketRuleSets.Num(); i++)
	{
		for (int j = 0; j < SocketRuleSets[i].Sockets.Num(); j++)
		{
			SocketDefinitions.Add(SocketRuleSets[i].Sockets[j]);
		}
	}

	TArray<FWFCTileDefinition> OriginalTiles = Tiles;

	for (const FWFCTileDefinition& OriginalTile : OriginalTiles)
	{
		if (!OriginalTile.bCanRotate) continue;

		for (int32 RotationSteps = 1; RotationSteps < 4; RotationSteps++)
		{
			FWFCTileDefinition RotatedTile = OriginalTile;
			RotatedTile.TileName = FString::Printf(TEXT("%s_Rot%d"),
			                                       *OriginalTile.TileName, RotationSteps * 90);

			RotatedTile.Sockets = RotateSockets(OriginalTile.Sockets, RotationSteps);
			RotatedTile.Category = OriginalTile.Category;
			RotatedTile.BaseRotation = FRotator(0, RotationSteps * 90.0f, 0);
			RotatedTile.bCanRotate = false;
			RotatedTile.MaxInstancesPerGeneration = OriginalTile.MaxInstancesPerGeneration;
			RotatedTile.bRequiresSupport = OriginalTile.bRequiresSupport;

			Tiles.Add(RotatedTile);
		}
	}
	for (int i = 0; i < Tiles.Num(); i++)
	{
		for (int d = 0; d < 6; d++)
		{
			for (int j = 0; j < Tiles.Num(); j++)
			{
				
				if (AreSocketsCompatible(Tiles[i].Sockets[d], Tiles[j].Sockets[d]))
				{
					int32 index;
					if (HasSocket(Tiles[i].Sockets[d], index))
					{
						if (!SocketDefinitions[index].CompatibleSockets.Contains(Tiles[j].Sockets[d]))
							SocketDefinitions[index].CompatibleSockets.Add(Tiles[j].Sockets[d]);
					}
					else
					{
						FWFCSocket newsocket;
						newsocket.SocketName = Tiles[i].Sockets[d];
						newsocket.CompatibleSockets.Add(Tiles[j].Sockets[d]);
						SocketDefinitions.Add(newsocket);
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("WFCTileSet: Generated rotation variants, total tiles: %d"), Tiles.Num());
}

TArray<FString> UWFCTileSet::RotateSockets(const TArray<FString>& OriginalSockets, int32 RotationSteps) const
{
	if (OriginalSockets.Num() != 6) return OriginalSockets;

	TArray<FString> RotatedSockets;
	RotatedSockets.SetNum(6);


	int32 HorizontalMapping[4] = {2, 5, 3, 4};
	//旋转水平方向的socket，仅仅是调换它们的位置
	for (int32 i = 0; i < 4; i++)
	{
		int32 OriginalIndex = HorizontalMapping[i];
		int32 RotatedIndex = HorizontalMapping[(i + RotationSteps) % 4];
		RotatedSockets[RotatedIndex] = OriginalSockets[OriginalIndex];
	}

	
	//旋转垂直方向的Socket
	//如果垂直方向是0，那么不作操作
	if (OriginalSockets[0].StartsWith("v") && OriginalSockets[0].Contains("_"))
	{
		RotatedSockets[0] = OriginalSockets[0];
		RotatedSockets[0][RotatedSockets[0].Len() - 1] = RotationSteps + '0';
	}
	else if (OriginalSockets[0].Equals("0") || OriginalSockets[0].Equals("-1"))
	{
		RotatedSockets[0] = OriginalSockets[0];
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Found invalid vertical socket define on dir %s, socket name: %s"), TEXT("Up"),
	   *OriginalSockets[0]);
	}

	if (OriginalSockets[1].StartsWith("v") && OriginalSockets[1].Contains("_"))
	{
		RotatedSockets[1] = OriginalSockets[1];
		RotatedSockets[1][RotatedSockets[1].Len() - 1] = RotationSteps + '0';
	}
	else if (OriginalSockets[1].Equals("0") || OriginalSockets[1].Equals("-1"))
	{
		RotatedSockets[1] = OriginalSockets[1];
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Found invalid vertical socket define on dir %s, socket name: %s"), TEXT("Down"),
		       *OriginalSockets[1]);
	}

	return RotatedSockets;
}

bool UWFCTileSet::HasSocket(const FString& SocketName, int32& outIndex) const
{
	for (int i = 0; i < SocketDefinitions.Num(); i++)
	{
		if (SocketDefinitions[i].SocketName.Equals(SocketName))
		{
			outIndex = i;
			return true;
		}
	}
	return false;
}