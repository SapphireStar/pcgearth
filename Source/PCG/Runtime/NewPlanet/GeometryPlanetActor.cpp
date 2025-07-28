// Fill out your copyright notice in the Description page of Project Settings.


#include "GeometryPlanetActor.h"

#include "MineSphere.h"
#include "MineSphereOre.h"
#include "MineSphereStone.h"
#include "NoiseApplier.h"
#include "RHICommandList.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshDeformFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "Rendering/Texture2DResource.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
AGeometryPlanetActor::AGeometryPlanetActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	DynamicMeshComponent = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("DynamicMeshComponent"));
	SetRootComponent(DynamicMeshComponent);
	PlanetSphereStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlanetSphere"));
	PlanetSphereStaticMesh->AttachToComponent(DynamicMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
	PlanetSphereStaticMesh->SetRelativeLocation(FVector(0, 0, 0));
	PlanetSphereStaticMesh->SetRelativeRotation(FRotator(0, 0, 0));	
}

// Called when the game starts or when spawned
void AGeometryPlanetActor::BeginPlay()
{
	Super::BeginPlay();
	
	/*MineSpheres.Empty();
	SpawnMineSpheres();
	UpdateMineAreas();*/
}

// Called every frame
void AGeometryPlanetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bIsTextureInitialized)
	{
		UpdateTexture16Bytes();
		if (DynamicMaterialInstance && DynamicTexture)
		{
			DynamicMaterialInstance->SetTextureParameterValue("SpherePos", DynamicTexture);
			DynamicMaterialInstance->SetScalarParameterValue("PlanetRadius", PlanetRadius * 1000);
			DynamicMaterialInstance->SetScalarParameterValue("DataCount", TextureDataSize);
		}
			bIsTextureInitialized = true;
	}
}

void AGeometryPlanetActor::InitializePlanet(FGeometryPlanetData PlanetData)
{
	PlanetRadius = PlanetData.PlanetRadius;
	PlanetResolution = PlanetData.PlanetResolution;
	PlanetMaterial = PlanetData.PlanetMaterial;
	DynamicMeshComponent->SetMaterial(0,PlanetMaterial);
	PlanetSphereStaticMesh->SetStaticMesh(PlanetData.PlanetSphereStaticMesh);
	RandomStream = FRandomStream(PlanetData.RandomSeed);
	MineConfiguration =  PlanetData.MineConfiguration;
	CraterSpawnConfiguration =  PlanetData.CraterConfiguration;
	FoliageAmount = PlanetData.FoliageAmount;
	bShouldSpawnFoliage = PlanetData.bShouldSpawnFoliage;
	MineConfiguration.MaxMineSphereAmount = PlanetData.MineConfiguration.MaxMineSphereAmount;
	MineSpheres.Empty();
}

int AGeometryPlanetActor::GetNextRandomAvaiableVertexID()
{
	return -1;
}

void AGeometryPlanetActor::ShuffleVertexID()
{
	FGeometryScriptIndexList VerticesList;
	bool bHasGaps = false;
	UGeometryScriptLibrary_MeshQueryFunctions::GetAllVertexIDs(DynamicMeshComponent->GetDynamicMesh(), VerticesList, bHasGaps);
	TArray<int> VertexIDs = *VerticesList.List;
}

bool AGeometryPlanetActor::AddPlanetVertexType(EVertexType eType, int VertexID)
{
	if (!CheckPlanetVertexType(eType, VertexID))
	{
		VertexTypeData[VertexID] += static_cast<int>(eType);
		return true;
	}
	UE_LOG(LogTemp, Warning, TEXT("AddPlanetVertexType: Vertex type already exists"));
	return false;
}

bool AGeometryPlanetActor::CheckPlanetVertexType(EVertexType eType, int VertexID)
{
	if (VertexTypeData[VertexID] == 0)
	{
		if (eType == EVertexType::EVT_None)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	if ((VertexTypeData[VertexID] & static_cast<int>(eType)) == static_cast<int>(eType))
	{
		return true;
	}
	return false;
}

void AGeometryPlanetActor::MarkPlanetRefresh(bool bImmediate, bool bImmediateEventFrozen)
{
	UDynamicMeshComponent* Component = DynamicMeshComponent;
	if (Component == nullptr)
	{
		return;
	}
	
	bool bEnabledDeferredCollision = false;
	if (Component->bDeferCollisionUpdates == false)
	{
		Component->SetDeferredCollisionUpdatesEnabled(true, false);
		bEnabledDeferredCollision = true;
	}

	if (Component->GetDynamicMesh())
	{
		Component->GetDynamicMesh()->Reset();
	}
	
	GeneratePlanet((Component->GetDynamicMesh()));

	if (bEnabledDeferredCollision)
	{
		Component->SetDeferredCollisionUpdatesEnabled(false, true);
	}
}

void AGeometryPlanetActor::ApplyNoiseToPlanet()
{
	/*if (!NoiseShapeSettings.IsValid())
		return;
	if (!NoiseShapeGenerator)
	{
		NoiseShapeGenerator = NewObject<UShapeGenerator>();
		NoiseShapeGenerator->Initialize(NoiseShapeSettings);
	}
	NoiseApplier::ApplySimpleNoise(DynamicMeshComponent->GetDynamicMesh(), FGeometryScriptMeshSelection(), nullptr, NoiseShapeGenerator);*/
}

void AGeometryPlanetActor::SpawnCraters()
{
	FGeometryScriptIndexList VerticesList;
	bool bHasGaps = false;
	UGeometryScriptLibrary_MeshQueryFunctions::GetAllVertexIDs(DynamicMeshComponent->GetDynamicMesh(), VerticesList, bHasGaps);
	TArray<int> VertexIDs = *VerticesList.List;
	
}

void AGeometryPlanetActor::ApplyCraterToPlanet()
{
	FGeometryScriptIndexList VerticesList;
	bool bHasGaps = false;
	UGeometryScriptLibrary_MeshQueryFunctions::GetAllVertexIDs(DynamicMeshComponent->GetDynamicMesh(), VerticesList, bHasGaps);
	TArray<int> VertexIDs = *VerticesList.List;
	for (int i = 0; i < VertexIDs.Num(); i++)
	{
		for (int c = 0; c < CratersData.Num(); c++)
		{
			bool bIsValidVertex;
			FVector VertexNewPos = NoiseApplier::ApplyCraterEffect(DynamicMeshComponent->GetDynamicMesh(), VertexIDs[i], GetActorLocation(), CratersData[c]);
			UGeometryScriptLibrary_MeshBasicEditFunctions::SetVertexPosition(DynamicMeshComponent->GetDynamicMesh(), VertexIDs[i], VertexNewPos, bIsValidVertex);
		}
		
	}
}

void AGeometryPlanetActor::SpawnStoneMineSpheres()
{
	UDynamicMesh* DynamicMesh = DynamicMeshComponent->GetDynamicMesh();
	int VertexCount  = DynamicMeshComponent->GetMesh()->VertexCount();
	int SpawnedMineSpheres = 0;
	for (int i = 0; i < VertexCount; i++)
	{
		if (SpawnedMineSpheres >= MineConfiguration.MaxMineSphereAmount)
		{
			break;
		}
		float shouldSpawnMineSphere = RandomStream.FRand();
		if (!(shouldSpawnMineSphere < 0.001))
		{
			continue;
		}
		bool isValidVertex = false;
		FVector VertexPosition = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
			DynamicMesh,
			i,
			isValidVertex);
		if (isValidVertex)
		{
			FVector Normal = VertexPosition.GetSafeNormal();
			FVector Position = VertexPosition + GetActorLocation();
			AMineSphereStone* MineSphere = GetWorld()->SpawnActor<AMineSphereStone>();
			MineSphere->UpdateMineSphere(RandomStream.FRandRange(MineConfiguration.RadiusMin, MineConfiguration.RadiusMax));
			MineSphere->SetMotherWorldPlanet(this);
			MineSphere->SetActorLocation(Position - Normal *  RandomStream.FRandRange(0, MineSphere->GetRadius() - 300));
			SpawnedMineSpheres++;
			MineSpheres.Add(MineSphere);
		}
	}
}

void AGeometryPlanetActor::SpawnOreMineSpheres()
{
	UDynamicMesh* DynamicMesh = DynamicMeshComponent->GetDynamicMesh();
	int VertexCount  = DynamicMeshComponent->GetMesh()->VertexCount();
	int SpawnedMineSpheres = 0;
	for (int i = 0; i < VertexCount; i++)
	{
		if (SpawnedMineSpheres >= MineConfiguration.MaxMineSphereAmount)
		{
			break;
		}
		float shouldSpawnMineSphere = RandomStream.FRand();
		if (!(shouldSpawnMineSphere < 0.001))
		{
			continue;
		}
		bool isValidVertex = false;
		FVector VertexPosition = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(
			DynamicMesh,
			i,
			isValidVertex);
		if (isValidVertex)
		{
			FVector Normal = VertexPosition.GetSafeNormal();
			FVector Position = VertexPosition + GetActorLocation();
			AMineSphereOre* MineSphere = GetWorld()->SpawnActor<AMineSphereOre>();
			MineSphere->UpdateMineSphere(RandomStream.FRandRange(MineConfiguration.RadiusMin, MineConfiguration.RadiusMax));
			MineSphere->SetMotherWorldPlanet(this);
			MineSphere->SetActorLocation(Position - Normal *  RandomStream.FRandRange(0, MineSphere->GetRadius() - 300));
			SpawnedMineSpheres++;
			MineSpheres.Add(MineSphere);
		}
	}
}

void AGeometryPlanetActor::GenerateMineAreas()
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
    
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this); 
    TArray<AActor*> ResultActors;
	bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		GetActorLocation(),
		PlanetRadius * 1000 + 1000,
		ObjectTypes,
		nullptr,
		ActorsToIgnore,
		ResultActors
	);
    if (bHit)
    {
	    for (int i = 0; i < ResultActors.Num(); i++)
	    {
		    if (auto mineralArea = Cast<AMineSphere>(ResultActors[i]))
		    {
		    	mineralArea->SetMotherWorldPlanet(this);
			    MineSpheres.Add(mineralArea);
		    }
	    }
    }
}

void AGeometryPlanetActor::GenerateMineMaterialTexture()
{
	while (TextureDataSize < MineSpheres.Num())
	{
		TextureDataSize *= 2;
	}
	//UGeometryScriptLibrary_MeshDeformFunctions::ApplyPerlinNoiseToMesh()
	if (MineSpheres.Num() == 0)
	{
		MinePositions.Init(FVector::ZeroVector, TextureDataSize);
		MineRadius.Init(0, TextureDataSize);
	}
	else
	{
		//如果mineral area小于最小datalength 4的话，需要补齐剩下的data内容，将冗余data置0
		MinePositions.Init(FVector::ZeroVector,TextureDataSize);
		MineRadius.Init(0, TextureDataSize);
		for (int i =0 ;i< MineSpheres.Num(); i++)
		{
			MinePositions[i] = (MineSpheres[i]->GetActorLocation());
			MineRadius[i] = (MineSpheres[i]->GetRadius());
		}
	}

	TextureWidth = TextureDataSize;
	DynamicMaterialInstance = UMaterialInstanceDynamic::Create(PlanetMaterial, this);
	DynamicMeshComponent->SetMaterial(0, DynamicMaterialInstance);
	InitializeTexture16Bytes();
	for (int i = 0; i < MinePositions.Num(); i++)
	{
		SetPixelValue(i, MinePositions[i].X, MinePositions[i].Y, MinePositions[i].Z, MineRadius[i]);
	}
	UpdateTexture16Bytes();

	bIsTextureInitialized = false;
}

void AGeometryPlanetActor::UpdateMineAreas()
{
	GenerateMineAreas();
	GenerateMineMaterialTexture();
}

void AGeometryPlanetActor::InitializeTexture16Bytes()
{
	TextureTotalPixels = TextureWidth * TextureHeight;

	TextureDataSize = TextureTotalPixels * 16;
	TextureDataSqrtSize = TextureWidth * 4;

	TextureDataFloat = new float[TextureTotalPixels * 4];

	DynamicTexture = UTexture2D::CreateTransient(TextureWidth, TextureHeight, PF_A32B32G32R32F);
	DynamicTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	DynamicTexture->SRGB = 0;
	//DynamicTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	DynamicTexture->Filter = TextureFilter::TF_Nearest;
	DynamicTexture->AddToRoot();
	DynamicTexture->UpdateResource();

	TextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, TextureWidth, TextureHeight);

	UpdateTexture16Bytes();
}

void AGeometryPlanetActor::UpdateTexture16Bytes(bool bFreeData)
{
	if (DynamicTexture == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dynamic Texture tried to Update Texture but it hasn't been initialized!"));
		return;
	}

	struct FUpdateTextureRegionsData
	{
		FTexture2DResource* Texture2DResource;
		FRHITexture* TextureRHI;
		int32 MipIndex;
		uint32 NumRegions;
		FUpdateTextureRegion2D* Regions;
		uint32 SrcPitch;
		uint32 SrcBpp;
		uint8* SrcData;
	};

	FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

	UTexture2D* Texture = DynamicTexture;

	RegionData->Texture2DResource = (FTexture2DResource*)Texture->GetResource();
	RegionData->TextureRHI = RegionData->Texture2DResource->GetTexture2DRHI();
	RegionData->MipIndex = 0;
	RegionData->NumRegions = 1;
	RegionData->Regions = TextureRegion;
	RegionData->SrcPitch = TextureDataSqrtSize;
	RegionData->SrcBpp = 16;
	RegionData->SrcData = (uint8*)TextureDataFloat;

	ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)(
		[RegionData, bFreeData, Texture](FRHICommandListImmediate& RHICmdList)
		{
			for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
			{
				int32 CurrentFirstMip = Texture->FirstResourceMemMip;
				if (RegionData->TextureRHI && RegionData->MipIndex >= CurrentFirstMip)
				{
					RHIUpdateTexture2D(
						RegionData->TextureRHI,
						RegionData->MipIndex - CurrentFirstMip,
						RegionData->Regions[RegionIndex],
						RegionData->SrcPitch,
						RegionData->SrcData
						+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
						+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
					);
				}
			}
			if (bFreeData)
			{
				FMemory::Free(RegionData->Regions);
				FMemory::Free(RegionData->SrcData);
			}
			delete RegionData;
		});
}

UDynamicMeshComponent* AGeometryPlanetActor::GetDynamicMeshComponent()
{
	return DynamicMeshComponent;
}


void AGeometryPlanetActor::SetPixelValue(int32 Offset, float X, float Y, float Z, float A)
{
	if (Offset < 0 || Offset >= TextureWidth)
	{
		return;
	}
	uint32 start = Offset * 4;

	TextureDataFloat[start] = X;
	TextureDataFloat[start + 1] = Y;
	TextureDataFloat[start + 2] = Z;
	TextureDataFloat[start + 3] = A;
}

void AGeometryPlanetActor::InitializeISMFoliage(UInstancedStaticMeshComponent* ISMComponent)
{
	ISMFoliageItemsHealth.Init(100, ISMComponent->GetInstanceCount());
}


void AGeometryPlanetActor::OnGetHitByLaser_Implementation(UInstancedStaticMeshComponent* ISMComponent, int32 ItemIndex, float Damage)
{
	OnISMInstanceHit.Broadcast(ISMComponent, ItemIndex, Damage);
}
