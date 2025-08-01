#include "TerrainBuildCrafterAbility.h"

#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PCG/Runtime/Factory/FactoryCrafter.h"
#include "PCG/Runtime/NewPlanet/GeometryPlanetActor.h"


UTerrainBuildCrafterAbility::UTerrainBuildCrafterAbility()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTerrainBuildCrafterAbility::OnInitializeAbility()
{
	Super::OnInitializeAbility();
}

void UTerrainBuildCrafterAbility::OnTickAbility()
{
	UItemAbilityComponent::OnTickAbility();
	if (bIsGridSlectionStarted)
	{
		FBox GridBounds = GridSelection->PeekGridSelection();
		FVector TargetPos = GridBounds.GetCenter();
		if (CheckCanBuildFactory(TargetPos, 0, GridBounds))
		{
			ChangeFactorySphereColor(AvailableColor);
		}
		else
		{
			ChangeFactorySphereColor(UnavailableColor);
		}
		FactorySphereMeshComponent->SetWorldLocation(TargetPos);
	}
}

void UTerrainBuildCrafterAbility::InitializeMineSphere()
{
	if (FactorySphereMesh && FactorySphereMaterial)
	{
		FactorySphereMeshComponent->SetStaticMesh(FactorySphereMesh);
		FactorySphereDynamicMaterial = UMaterialInstanceDynamic::Create(FactorySphereMaterial, this);
		FactorySphereMeshComponent->SetMaterial(0, FactorySphereDynamicMaterial);
		float MeshRadius = FactorySphereMeshComponent->GetStaticMesh()->GetBoundingBox().GetExtent().X;
		float MeshScale = PlayerData->GetPlayerData().CraftingFactoryInfo.FactoryRadius / MeshRadius;
		FactorySphereMeshComponent->SetWorldScale3D(FVector(MeshScale, MeshScale, MeshScale));
		FactorySphereMeshComponent->SetVisibility(false);
		FactorySphereMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void UTerrainBuildCrafterAbility::OnActivateAbility()
{
	Super::OnActivateAbility();
	SetFactoryRecipeInfo(PlayerData->GetPlayerCurrentRecipe());
}

void UTerrainBuildCrafterAbility::OnCompleteUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
	if (!TraceStartComp || !Camera)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid components for building"));
		return;
	}

	if (!GridSelection)
	{
		UE_LOG(LogTemp, Warning, TEXT("Grid Selection is invalid"));
		return;
	}

	if (!bIsGridSlectionStarted)
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
			if (AGeometryPlanetActor* planet = Cast<AGeometryPlanetActor>(HitResult.GetActor()))
			{
				SelectPlanet(planet, HitResult);
				GridSelection->StartGridSelection(HitResult.ImpactPoint,
				                                  FindNormalOnPlanet(HitResult.ImpactPoint, planet->GetActorLocation()),
				                                  FindNormalRotationOnPlanet(HitResult.ImpactPoint,
				                                                             planet->GetActorLocation()));
				bIsGridSlectionStarted = true;

				if (bShouldCheckNearFactory)
				{
					FactorySphereMeshComponent->SetVisibility(true);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Hit non-planet object, cannot build here"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No valid build target found"));
		}
	}
	else if (Planet)
	{
		FBox GridBounds = GridSelection->PeekGridSelection();
		if (ValidateGridBounds(GridBounds))
		{
			FIntVector GridSize = GridSelection->PeekGridSize();
			if (ProcessBuilding(Planet, LastHitResult, GridBounds, GridSize, nullptr))
			{
				bIsGridSlectionStarted = false;
				GridSelection->EndGridSelection();
				DeselectPlanet();
				FactorySphereMeshComponent->SetVisibility(false);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Process Terrain Build failed"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Grid selection is invalid"));
		}
	}
	else
	{
		DeselectPlanet();
		bIsGridSlectionStarted = false;
		UE_LOG(LogTemp, Warning, TEXT("No valid planet found"));
	}
}

void UTerrainBuildCrafterAbility::SpawnFactoryActor(FVector Position, int Volume, AMineSphere* MineSphere, float Radius)
{
	FactoryManager->BuildCraftFactoryAt(Position, Volume, PlayerData->GetPlayerData().CraftingFactoryInfo,
	                                    FactoryRecipeInfo);
}

float UTerrainBuildCrafterAbility::CalculateFactoryRadius(int Volume)
{
	return PlayerData->GetPlayerData().CraftingFactoryInfo.FactoryRadius;
}

void UTerrainBuildCrafterAbility::SetFactoryRecipeInfo(FFactoryRecipeInfo RecipeInfo)
{
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(
		                                  TEXT("SetFactoryRecipeInfo: Output %s"),
		                                  *UEnum::GetValueAsString(RecipeInfo.Output.ResourceType)));
	this->FactoryRecipeInfo = RecipeInfo;
}
