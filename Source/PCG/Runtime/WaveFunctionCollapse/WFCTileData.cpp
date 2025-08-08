// Fill out your copyright notice in the Description page of Project Settings.


#include "WFCTileData.h"

void FWFCTile::InitializeTileVariant(const FWFCTile& other, TArray<FString> Socket, float Rotationz)
{
	this->TileName = other.TileName + TEXT("_") + FString::SanitizeFloat(Rotationz);
	this->StaticMesh =  other.StaticMesh;
	this->Sockets =  Socket;
	this->bShouldCreateRotationVariant =  other.bShouldCreateRotationVariant;
	this->RotationZ =  Rotationz;
	this->Weight =  other.Weight;
	this->bIsBaseTile = false;
}
