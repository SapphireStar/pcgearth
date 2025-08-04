// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradeItemData.h"

FUpgradeItemInfo UUpgradeItemData::GetUpgradeItemInfo(EPlayerAbilityPropertyType UpgradeItemType) const
{
	for (int i = 0; i < UpgradeItemTypes.Num(); i++)
	{
		if (UpgradeItemTypes[i].UpgradeType == UpgradeItemType)
		{
			return UpgradeItemTypes[i];
		}
	}
	UE_LOG(LogTemp, Error, TEXT("UUpgradeItemData: No such upgrade type: %s"), *UEnum::GetValueAsString(UpgradeItemType));
	return FUpgradeItemInfo();
}
