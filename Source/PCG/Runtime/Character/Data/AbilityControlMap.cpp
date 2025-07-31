// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityControlMap.h"

FAbilityControlGroup UAbilityControlMap::GetControlGroup(EAbilityType eAbilityType)
{
	for (int i = 0; i < ControlMaps.Num(); i++)
	{
		if (eAbilityType == ControlMaps[i].AbilityType)
		{
			return ControlMaps[i];
		}
	}
	
	return DefaultControlGroup;
}
