// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemPlaceComponent.h"

#include "EngineUtils.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/InputSettings.h"
#include "Generators/StairGenerator.h"
#include "GeometryScript/GeometryScriptSelectionTypes.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PCG/Runtime/NewPlanet/GeometryPlanet.h"
#include "PCG/Runtime/WaveFunctionCollapse/WFCGenerator.h"


// Sets default values for this component's properties
UItemPlaceComponent::UItemPlaceComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UItemPlaceComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if (!WFCGenerator && GetWorld())
	{
		for (TActorIterator<AWFCGenerator> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (*ActorItr)
			{
				WFCGenerator = *ActorItr;
				WFCGenerator->AttachToActor(GetOwner(), FAttachmentTransformRules::KeepRelativeTransform);
			}
		}
	}
}


// Called every frame
void UItemPlaceComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UItemPlaceComponent::SelectPoint(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
		FVector End = TraceStartComp->GetComponentLocation() + Camera->GetForwardVector() * SelectRange;
	TArray<AActor*> ActorsToIgnore;
	FHitResult HitResult;
	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TraceStartComp->GetComponentLocation(),
		End,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		5.f);
	if (HitResult.bBlockingHit)
	{
		if (AGeometryPlanet* planet = Cast<AGeometryPlanet>(HitResult.GetActor()))
		{
			FVector ImpactRelativePoint = HitResult.ImpactPoint - planet->GetActorLocation();
			FGeometryScriptMeshSelection selection;
			UGeometryScriptLibrary_MeshSelectionFunctions::SelectMeshElementsInSphere(
				planet->GetDynamicMeshComponent()->GetDynamicMesh(),
				selection,
				ImpactRelativePoint,
				VertexSelectionTolerance,
				EGeometryScriptMeshSelectionType::Vertices,
				false,
				1
			);
			TArray<int32> indicesout;

			selection.ConvertToMeshIndexArray(
				planet->GetDynamicMeshComponent()->GetDynamicMesh()->GetMeshRef(),
				indicesout);


			/*bool bIsValidVertex;
			auto mesh = planet->GetDynamicMeshComponent()->GetDynamicMesh();
			int VertexID = FindVertex(HitResult.ImpactPoint, planet->GetDynamicMeshComponent());
			auto pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(mesh, VertexID, bIsValidVertex);

			FVector normal = (pos - planet->GetActorLocation());
			normal.Normalize();
			UGeometryScriptLibrary_MeshBasicEditFunctions::SetVertexPosition(
				mesh, VertexID, pos + normal * 100.f, bIsValidVertex);
			
			pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(mesh, VertexID, bIsValidVertex);
			UE_LOG(LogTemp, Warning, TEXT("impact point: %f, %f, %f"), HitResult.ImpactPoint.X, HitResult.ImpactPoint.Y, HitResult.ImpactPoint.Z);
			UE_LOG(LogTemp, Warning, TEXT("index after change: %f, %f, %f"), pos.X, pos.Y, pos.Z);
			planet->GetDynamicMeshComponent()->NotifyMeshUpdated();*/


			if (indicesout.Num() > 0)
			{
				int LowestVertexID = FindLowestVertex(planet->GetDynamicMeshComponent(), indicesout);
				bool bIsValidVertex;
				auto mesh = planet->GetDynamicMeshComponent()->GetDynamicMesh();

				auto lowesetPos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
					mesh, LowestVertexID, bIsValidVertex);
				float lowestLength = (lowesetPos).Length();
				for (int i : indicesout)
				{
					if (i != LowestVertexID)
					{
						auto pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
							mesh, i, bIsValidVertex);
						FVector normal = (pos);
						normal.Normalize();
						UGeometryScriptLibrary_MeshBasicEditFunctions::SetVertexPosition(
							mesh, i, normal * lowestLength, bIsValidVertex);
					}
				}

				planet->GetDynamicMeshComponent()->NotifyMeshUpdated();
				planet->GetDynamicMeshComponent()->UpdateCollision();

				//临时用于让建筑与星球垂直
				FVector normal = HitResult.ImpactPoint - HitResult.GetActor()->GetActorLocation();
				normal.Normalize();
				FRotator rotation = UKismetMathLibrary::FindLookAtRotation(HitResult.GetActor()->GetActorLocation(),
																		   HitResult.GetActor()->GetActorLocation() + normal);

				GenerateBuilding(5, 6, 7, (HitResult.ImpactPoint - planet->GetActorLocation()).GetSafeNormal() * lowestLength + planet->GetActorLocation(), rotation);

				/*int ClosestVertexID = FindVertex(HitResult.ImpactPoint, planet->GetDynamicMeshComponent(),indicesout);

				
				bool bIsValidVertex;
				auto mesh = planet->GetDynamicMeshComponent()->GetDynamicMesh();
				auto pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
					mesh, ClosestVertexID, bIsValidVertex);
				UE_LOG(LogTemp, Warning, TEXT("index: %f, %f, %f"), pos.X, pos.Y, pos.Z);
				UE_LOG(LogTemp, Warning, TEXT("impact pos: %f, %f, %f"), HitResult.ImpactPoint.X,
				       HitResult.ImpactPoint.Y, HitResult.ImpactPoint.Z);
				FVector normal = (pos - planet->GetActorLocation());
				normal.Normalize();

				UGeometryScriptLibrary_MeshBasicEditFunctions::SetVertexPosition(
					mesh, ClosestVertexID, pos + normal * 100.f, bIsValidVertex);
				pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(mesh, ClosestVertexID, bIsValidVertex);
				UE_LOG(LogTemp, Warning, TEXT("index after change: %f, %f, %f"), pos.X, pos.Y, pos.Z);
				planet->GetDynamicMeshComponent()->NotifyMeshUpdated();
				planet->GetDynamicMeshComponent()->UpdateCollision();*/
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No selection found"));
			}
		}


	}
}

void UItemPlaceComponent::DigTerrain(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
			FVector End = TraceStartComp->GetComponentLocation() + Camera->GetForwardVector() * SelectRange;
	TArray<AActor*> ActorsToIgnore;
	FHitResult HitResult;
	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TraceStartComp->GetComponentLocation(),
		End,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		5.f);
	if (HitResult.bBlockingHit)
	{
		if (AGeometryPlanet* planet = Cast<AGeometryPlanet>(HitResult.GetActor()))
		{
			FVector ImpactRelativePoint = HitResult.ImpactPoint - planet->GetActorLocation();
			FGeometryScriptMeshSelection selection;
			UGeometryScriptLibrary_MeshSelectionFunctions::SelectMeshElementsInSphere(
				planet->GetDynamicMeshComponent()->GetDynamicMesh(),
				selection,
				ImpactRelativePoint,
				VertexSelectionTolerance,
				EGeometryScriptMeshSelectionType::Vertices,
				false,
				1
			);
			TArray<int32> indicesout;

			selection.ConvertToMeshIndexArray(
				planet->GetDynamicMeshComponent()->GetDynamicMesh()->GetMeshRef(),
				indicesout);

			if (indicesout.Num() > 0)
			{
				int LowestVertexID = FindLowestVertex(planet->GetDynamicMeshComponent(), indicesout);
				bool bIsValidVertex;
				auto mesh = planet->GetDynamicMeshComponent()->GetDynamicMesh();

				auto lowesetPos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
					mesh, LowestVertexID, bIsValidVertex);
				float lowestLength = (lowesetPos).Length();
				for (int i : indicesout)
				{
					if (i != LowestVertexID)
					{
						auto pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
							mesh, i, bIsValidVertex);
						FVector normal = (pos);
						normal.Normalize();
						UGeometryScriptLibrary_MeshBasicEditFunctions::SetVertexPosition(
							mesh, i, normal * (lowestLength - 100.f), bIsValidVertex);
					}
				}

				planet->GetDynamicMeshComponent()->NotifyMeshUpdated();
				planet->GetDynamicMeshComponent()->UpdateCollision();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No selection found"));
			}
		}


	}
}

void UItemPlaceComponent::GenerateBuilding(int SizeX, int SizeY, int SizeZ, const FVector& Location,
                                           const FRotator& Rotation)
{
	if (!WFCGenerator) return;
	WFCGenerator->StartWFC(SizeX, SizeY, SizeZ, Location, Rotation);
}

int UItemPlaceComponent::FindVertex(const FVector& Target, UDynamicMeshComponent* DynamicMeshComp,
	TArray<int32> VertexID)
{
	auto DynamicMesh = DynamicMeshComp->GetDynamicMesh();
	bool bHasIdGroup;

	float min = FLT_MAX;
	int minID = 0;
	for (auto id : VertexID)
	{
		FVector pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(DynamicMesh, id, bHasIdGroup);
		if (FVector::Dist(pos, Target) < min)
		{
			min = FVector::Dist(pos, Target);
			minID = id;
		}
	}
	return minID;
}

int UItemPlaceComponent::FindLowestVertex(UDynamicMeshComponent* DynamicMeshComp, TArray<int32> VertexID)
{
	auto DynamicMesh = DynamicMeshComp->GetDynamicMesh();
	bool bHasIdGroup;
	FVector planetPos = DynamicMeshComp->GetOwner()->GetActorLocation();

	float min = FLT_MAX;
	int minID = 0;
	for (auto id : VertexID)
	{
		FVector pos = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(DynamicMesh, id, bHasIdGroup);
		float dist = FVector::Dist(pos, planetPos);
		if (dist < min)
		{
			min = dist;
			minID = id;
		}
	}
	return minID;
}

