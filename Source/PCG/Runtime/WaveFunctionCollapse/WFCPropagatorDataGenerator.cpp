// Fill out your copyright notice in the Description page of Project Settings.
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
	GenerateCompleteData();
	BuildPropagatorTable();
	outPropagator.Empty();
	outPropagator.SetNum(6);
	for (int d = 0; d < 6; d++)
	{
		outPropagator[d].SetNum(Propagator[d].Num());
		for (int t = 0; t < Propagator[d].Num(); t++)
		{
			for (int otherT = 0; otherT < Propagator[d][t].Num(); otherT++)
			{
				outPropagator[d][t].Add(Propagator[d][t][otherT]);
			}
		}
	}
	return true;
}

int UWFCPropagatorDataGenerator::GetTileCount() const
{
	return BaseTileData->Tiles.Num();
}

const FWFCTile UWFCPropagatorDataGenerator::GetTile(int TileID) const
{
	return  BaseTileData->Tiles[TileID];
}

UWFCTileData* UWFCPropagatorDataGenerator::GetCompleteData() const
{
	return CompleteTileData;
}

void UWFCPropagatorDataGenerator::GenerateCompleteData()
{
	CompleteTileData = NewObject<UWFCTileData>();
	for (int i = 0; i < BaseTileData->Tiles.Num(); i++)
	{
		CompleteTileData->Tiles.Add(BaseTileData->Tiles[i]);
		if (ShouldCreateRotationVariants(BaseTileData->Tiles[i]))
		{
			for (int r = 0; r < 3; r++)
			{
				FWFCTile RotatedTile;
				TArray<FString> RotatedTileNames = RotateSocketsZ90(BaseTileData->Tiles[i].Sockets);
				RotatedTile.InitializeTileVariant(BaseTileData->Tiles[i], RotatedTileNames, 90.f * (r + 1));
				CompleteTileData->Tiles.Add(RotatedTile);
			}
		}
	}
}

bool UWFCPropagatorDataGenerator::ShouldCreateRotationVariants(const FWFCTile& TileInfo)
{
	return TileInfo.bShouldCreateRotationVariant;
}

TArray<FString> UWFCPropagatorDataGenerator::RotateSocketsZ90(const TArray<FString>& OriginalSockets)
{
	TArray<FString> RotatedSockets;
	RotatedSockets.SetNum(6);
	RotatedSockets[0] = OriginalSockets[0];
	RotatedSockets[1] = OriginalSockets[1];

	RotatedSockets[2] = OriginalSockets[5];
	RotatedSockets[3] = OriginalSockets[4];
	RotatedSockets[4] = OriginalSockets[2];
	RotatedSockets[5] = OriginalSockets[3];
	return RotatedSockets;
}

void UWFCPropagatorDataGenerator::BuildPropagatorTable()
{
	if (!CompleteTileData)
	{
		UE_LOG(LogTemp, Error, TEXT("UWFCPropagatorDataGenerator::BuildPropagatorTable: Complete data not generated"));
		return;
	}
	int TileCount = CompleteTileData->Tiles.Num();

	Propagator.Empty();
	Propagator.SetNum(6);

	for (int d = 0; d < 6; d++)
	{
		Propagator[d].SetNum(CompleteTileData->Tiles.Num());
		for (int t = 0; t < CompleteTileData->Tiles.Num(); t++)
		{
			int oppositeDir = GetOppositeDirection(d);
			FString CurrentSocket = CompleteTileData->Tiles[t].GetSocketForDirection(d);
			TArray<FString> CompatibleSockets = SocketData->GetCompatibleSockets(CurrentSocket);

			for (int otherT = 0;  otherT < CompleteTileData->Tiles.Num(); otherT++)
			{
				const FWFCTile& OtherTile = CompleteTileData->Tiles[otherT];
				FString OtherSocket = CompleteTileData->Tiles[otherT].GetSocketForDirection(oppositeDir);
				
				if (SocketData->AreSocketsCompatible(CurrentSocket, OtherSocket))
				{
					Propagator[d][t].Add(otherT);
				}
				
			}
		}
	}
	
}

int UWFCPropagatorDataGenerator::GetOppositeDirection(int Direction)
{
	switch (Direction)
	{
	case 0:
		return 1;
	case 1:
		return 0;
	case 2:
		return 3;
	case 3:
		return 2;
	case 4:
		return 5;
	case 5:
		return 4;
	}
	UE_LOG(LogTemp, Error, TEXT("UWFCPropagatorDataGenerator::GetOppositeDirection: Error Direction: %d"), Direction);
	return -1;
}
