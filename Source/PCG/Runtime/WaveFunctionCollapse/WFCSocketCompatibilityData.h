// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WFCSocketCompatibilityData.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FSocketCompatibility
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FString> CompatibleSockets;
};
UCLASS()
class PCG_API UWFCSocketCompatibilityData : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSocketCompatibility> Sockets;
	
public:
	UFUNCTION(BlueprintCallable)
	TArray<FString> GetCompatibleSockets(const FString& SocketName) const;

	UFUNCTION(BlueprintCallable)
	bool AreSocketsCompatible(const FString& Socket1, const FString& Socket2) const;

	UFUNCTION(BlueprintCallable)
	bool ValidateSocketRules() const;
};
