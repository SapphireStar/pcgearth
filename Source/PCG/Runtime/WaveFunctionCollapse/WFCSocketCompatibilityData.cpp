// WFCSocketCompatibilityData.cpp - Improved version with better validation

#include "WFCSocketCompatibilityData.h"

TArray<FString> UWFCSocketCompatibilityData::GetCompatibleSockets(const FString& SocketName) const
{
	// 处理空socket名称
	if (SocketName.IsEmpty() || SocketName == TEXT("Empty") || SocketName == TEXT("None"))
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("UWFCSocketCompatibilityData: Empty socket name requested"));
		return TArray<FString>();
	}
	
	for (int i = 0; i < Sockets.Num(); i++)
	{
		if (SocketName.Equals(Sockets[i].SocketName, ESearchCase::IgnoreCase))
		{
			UE_LOG(LogTemp, VeryVerbose, TEXT("UWFCSocketCompatibilityData: Found %d compatible sockets for '%s'"), 
				Sockets[i].CompatibleSockets.Num(), *SocketName);
			return Sockets[i].CompatibleSockets;
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("UWFCSocketCompatibilityData::GetCompatibleSockets: Can't find socket name: '%s'"), *SocketName);
	return TArray<FString>();
}

bool UWFCSocketCompatibilityData::AreSocketsCompatible(const FString& Socket1, const FString& Socket2) const
{
	// 处理空socket
	if (Socket1.IsEmpty() || Socket2.IsEmpty() || 
		Socket1 == TEXT("Empty") || Socket2 == TEXT("Empty") ||
		Socket1 == TEXT("None") || Socket2 == TEXT("None"))
	{
		// 空socket只与空socket兼容
		bool bothEmpty = (Socket1.IsEmpty() || Socket1 == TEXT("Empty") || Socket1 == TEXT("None")) &&
		                 (Socket2.IsEmpty() || Socket2 == TEXT("Empty") || Socket2 == TEXT("None"));
		
		UE_LOG(LogTemp, VeryVerbose, TEXT("UWFCSocketCompatibilityData: Empty socket compatibility check: '%s' vs '%s' = %s"), 
			*Socket1, *Socket2, bothEmpty ? TEXT("true") : TEXT("false"));
		
		return bothEmpty;
	}
	
	// 检查双向兼容性
	auto Socket1Compatibles = GetCompatibleSockets(Socket1);
	auto Socket2Compatibles = GetCompatibleSockets(Socket2);
	
	bool compatible = Socket1Compatibles.Contains(Socket2) || Socket2Compatibles.Contains(Socket1);
	
	UE_LOG(LogTemp, VeryVerbose, TEXT("UWFCSocketCompatibilityData: Socket compatibility check: '%s' vs '%s' = %s"), 
		*Socket1, *Socket2, compatible ? TEXT("true") : TEXT("false"));
	
	return compatible;
}

bool UWFCSocketCompatibilityData::ValidateSocketRules() const
{
	UE_LOG(LogTemp, Log, TEXT("UWFCSocketCompatibilityData: Validating socket rules..."));
	
	if (Sockets.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWFCSocketCompatibilityData: No socket rules defined"));
		return false;
	}
	
	bool hasErrors = false;
	TSet<FString> definedSockets;
	
	// 收集所有定义的socket名称
	for (const auto& socketRule : Sockets)
	{
		if (socketRule.SocketName.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("UWFCSocketCompatibilityData: Found socket rule with empty name"));
			hasErrors = true;
			continue;
		}
		
		if (definedSockets.Contains(socketRule.SocketName))
		{
			UE_LOG(LogTemp, Error, TEXT("UWFCSocketCompatibilityData: Duplicate socket name found: '%s'"), 
				*socketRule.SocketName);
			hasErrors = true;
		}
		
		definedSockets.Add(socketRule.SocketName);
	}
	
	// 验证兼容性规则
	for (const auto& socketRule : Sockets)
	{
		for (const FString& compatibleSocket : socketRule.CompatibleSockets)
		{
			// 检查兼容的socket是否已定义
			if (!definedSockets.Contains(compatibleSocket) && 
				compatibleSocket != TEXT("Empty") && 
				compatibleSocket != TEXT("None") &&
				!compatibleSocket.IsEmpty())
			{
				UE_LOG(LogTemp, Warning, TEXT("UWFCSocketCompatibilityData: Socket '%s' references undefined compatible socket '%s'"), 
					*socketRule.SocketName, *compatibleSocket);
			}
			
			// 检查对称性：如果A兼容B，B也应该兼容A
			bool foundReverse = false;
			for (const auto& otherRule : Sockets)
			{
				if (otherRule.SocketName == compatibleSocket)
				{
					if (otherRule.CompatibleSockets.Contains(socketRule.SocketName))
					{
						foundReverse = true;
						break;
					}
				}
			}
			
			if (!foundReverse && compatibleSocket != socketRule.SocketName) // 自兼容不需要检查对称性
			{
				UE_LOG(LogTemp, Warning, TEXT("UWFCSocketCompatibilityData: Asymmetric compatibility: '%s' -> '%s' but not reverse"), 
					*socketRule.SocketName, *compatibleSocket);
			}
		}
		
		UE_LOG(LogTemp, VeryVerbose, TEXT("UWFCSocketCompatibilityData: Socket '%s' has %d compatible sockets"), 
			*socketRule.SocketName, socketRule.CompatibleSockets.Num());
	}
	
	if (!hasErrors)
	{
		UE_LOG(LogTemp, Log, TEXT("UWFCSocketCompatibilityData: Socket rules validation passed"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UWFCSocketCompatibilityData: Socket rules validation failed"));
	}
	
	return !hasErrors;
}
