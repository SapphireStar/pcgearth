// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceData.h"

FResourceInfo UResourceData::GetResourceInfo(EFactoryResource ResourceType) const
{
	for (int i = 0; i < ResourceTypes.Num(); i++)
	{
		if (ResourceTypes[i].ResourceType == ResourceType)
		{
			return ResourceTypes[i];
		}
	}
	UE_LOG(LogTemp, Error, TEXT("UResourceData: No such resource type: %s"), *UEnum::GetValueAsString(ResourceType));
	return FResourceInfo();
}
