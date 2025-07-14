// Fill out your copyright notice in the Description page of Project Settings.


#include "WFCTileData.h"

void FWFCTile::InitializeTileVariant(const FWFCTile& other, TArray<FString> Sockets, float RotationZ)
{
	this->TileName = other.TileName + TEXT("_") + FString::SanitizeFloat(RotationZ);
	this->StaticMesh =  other.StaticMesh;
	this->Sockets =  Sockets;
	this->bShouldCreateRotationVariant =  other.bShouldCreateRotationVariant;
	this->RotationZ =  RotationZ;
	this->Weight =  other.Weight;
	this->bIsBaseTile = false;
}
