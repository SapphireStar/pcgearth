// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityData.h"

FAbilityInfo UAbilityData::GetAbilityInfo(EAbilityType AbilityType) const
{
	for (int i = 0; i < Abilities.Num(); i++)
	{
		if (AbilityType == Abilities[i].AbilityType)
		{
			return Abilities[i];
		}
	}
	UE_LOG(LogTemp, Error, TEXT("UAbilityData::GetAbilityInfo: Ability type %d not found"),
	       *UEnum::GetValueAsString(AbilityType));
	return FAbilityInfo();
}

FAbilityInfo UAbilityData::GetAbilityCanProduceResource(EFactoryResource ResourceType)
{
	for (int i = 0; i < Abilities.Num(); i++)
	{
		for (int j = 0; j < Abilities[i].CanProduceResourceType.Num(); j++)
		{
			if (Abilities[i].CanProduceResourceType[j] == ResourceType)
			{
				return Abilities[i];
			}
		}
	}
	return FAbilityInfo();
}
