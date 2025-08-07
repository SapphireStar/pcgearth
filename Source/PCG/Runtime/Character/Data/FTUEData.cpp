// Fill out your copyright notice in the Description page of Project Settings.


#include "FTUEData.h"

FTutorialChainData UFTUEData::GetTutorialChain(ETutorialType eType)
{
	for (int i = 0; i < TutorialChains.Num(); i++)
	{
		if (TutorialChains[i].TutorialType == eType)
		{
			return TutorialChains[i];
		}
	}

	UE_LOG(LogTemp, Error, TEXT("UFTUEData::GetTutorialChain: no available Tutorial Chaine for Type: %s"), *UEnum::GetValueAsString(eType));
	return FTutorialChainData();
}
