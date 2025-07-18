// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemPlaceComponent.generated.h"

struct FInputActionValue;
class UCameraComponent;
class UDynamicMeshComponent;
class AWFCGenerator;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PCG_API UItemPlaceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UItemPlaceComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	void SelectPoint(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera);
	void DigTerrain(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera);
private:
	void GenerateBuilding(int SizeX, int SizeY, int SizeZ, const FVector& Location, const FRotator& Rotation);
	int FindVertex(const FVector& Target, UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID);
	int FindLowestVertex(UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID);
protected:
	//WFC
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AWFCGenerator> WFCGenerator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VertexSelectionTolerance = 500.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SelectRange = 10000.f;
};
