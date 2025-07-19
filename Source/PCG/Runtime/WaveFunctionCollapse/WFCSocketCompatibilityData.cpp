// Fill out your copyright notice in the Description page of Project Settings.


#include "WFCSocketCompatibilityData.h"

TArray<FString> UWFCSocketCompatibilityData::GetCompatibleSockets(const FString& SocketName) const
{
	for (int i = 0; i < Sockets.Num(); i++)
	{
		if (SocketName.Equals(Sockets[i].SocketName))
		{
			return Sockets[i].CompatibleSockets;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("UWFCSocketCompatibilityData::GetCompatibleSockets: Can't find socket name: %s"), *SocketName);
	return TArray<FString>();
}

bool UWFCSocketCompatibilityData::AreSocketsCompatible(const FString& Socket1, const FString& Socket2) const
{
	auto S1 = GetCompatibleSockets(Socket1);
	if (S1.Contains(Socket2))
		return true;
	return false;
}

bool UWFCSocketCompatibilityData::ValidateSocketRules() const
{
	return true;
}
