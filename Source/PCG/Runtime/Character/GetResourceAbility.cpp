
#include "GetResourceAbility.h"

#include "SpaceShipPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PCG/Runtime/NewPlanet/GeometryPlanetActor.h"


// Sets default values for this component's properties
UGetResourceAbility::UGetResourceAbility()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGetResourceAbility::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UGetResourceAbility::OnInitializeAbility()
{
	Super::OnInitializeAbility();
}

void UGetResourceAbility::OnActivateAbility()
{
	Super::OnActivateAbility();
	if (ASpaceShipPawn* Player = Cast<ASpaceShipPawn>(GetOwner()))
	{
		Range = Player->GetLaserRange();
	}
}

void UGetResourceAbility::OnTickAbility()
{
	Super::OnTickAbility();
}

void UGetResourceAbility::OnDeactivateAbility()
{
	Super::OnDeactivateAbility();
}

void UGetResourceAbility::OnStartUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
	Super::OnStartUseAbility(TraceStartComp, Camera);
}

void UGetResourceAbility::OnKeepUsingAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
	Super::OnKeepUsingAbility(TraceStartComp, Camera);
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(GetOwner());
	TraceParams.bTraceComplex = false;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult HitResult;
	FVector End = Camera->GetComponentLocation() + Camera->GetForwardVector() * Range;


	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Camera->GetComponentLocation(),
		End,
		ECC_Visibility,
		TraceParams
		);

	if (bHit)
	{
		if (AGeometryPlanetActor* Planet = Cast<AGeometryPlanetActor>(HitResult.GetActor()))
		{
			float PlayerBaseDamage = PlayerData->GetPlayerAbilityPropertyValue(EPlayerAbilityPropertyType::EPAPT_Damage);
			float PlayerDamageMultiplier = PlayerData->GetPlayerAbilityPropertyValue(EPlayerAbilityPropertyType::EPAPT_DamageMultiplier);
			if (UInstancedStaticMeshComponent* ISM= Cast<UInstancedStaticMeshComponent>(HitResult.Component))
				Planet->OnGetHitByLaser(ISM, HitResult.Item, PlayerBaseDamage * PlayerDamageMultiplier * GetWorld()->DeltaTimeSeconds);
		}
	}	
}

void UGetResourceAbility::OnCompleteUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera)
{
	Super::OnCompleteUseAbility(TraceStartComp, Camera);
}

void UGetResourceAbility::SetLaserRange(float range)
{
	Range = range;
}
