// Fill out your copyright notice in the Description page of Project Settings.


#include "GeometryPlanet.h"
#include "RHICommandList.h"
#include "Rendering/Texture2DResource.h"
#include "GeometryScript/MeshDeformFunctions.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/MeshSelectionFunctions.h"
#include "GeometryScript/MeshSubdivideFunctions.h"


// Sets default values
AGeometryPlanet::AGeometryPlanet()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PlanetSphereStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlanetSphere"));
	PlanetSphereStaticMesh->AttachToComponent(DynamicMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
	PlanetSphereStaticMesh->SetRelativeLocation(FVector(0, 0, 0));
	PlanetSphereStaticMesh->SetRelativeRotation(FRotator(0, 0, 0));
}

// Called when the game starts or when spawned
void AGeometryPlanet::BeginPlay()
{
	Super::BeginPlay();
	GenerateMineMaterialTexture();
}

// Called every frame
void AGeometryPlanet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bIsTextureInitialized)
	{
		UpdateTexture16Bytes();
		if (DynamicMaterialInstance && DynamicTexture)
		{
			
			DynamicMaterialInstance->SetTextureParameterValue("SpherePos", DynamicTexture);
			DynamicMaterialInstance->SetScalarParameterValue("DataCount", TextureDataSize);
		}
			
	}
}

void AGeometryPlanet::RebuildGeneratedMesh(UDynamicMesh* TargetMesh)
{
	Super::RebuildGeneratedMesh(TargetMesh);
	GeneratePlanet(TargetMesh);
}

void AGeometryPlanet::MarkPlanetRefresh(bool bImmediate, bool bImmediateEventFrozen)
{
	MarkForMeshRebuild(bImmediate, bImmediateEventFrozen);
}

void AGeometryPlanet::GenerateMineAreas()
{
}

void AGeometryPlanet::GenerateMineMaterialTexture()
{
	while (TextureDataSize < MinePositions.Num())
	{
		TextureDataSize *= 2;
	}
	DynamicMaterialInstance = UMaterialInstanceDynamic::Create(Material, this);
	DynamicMeshComponent->SetMaterial(0, DynamicMaterialInstance);
	InitializeTexture16Bytes();
	for (int i = 0; i < MinePositions.Num(); i++)
	{
		SetPixelValue(i, MinePositions[i].X, MinePositions[i].Y, MinePositions[i].Z, MineRadius[i]);
	}
	UpdateTexture16Bytes();
}

void AGeometryPlanet::InitializeTexture16Bytes()
{
	// Get Total Pixels in Texture
	TextureTotalPixels = TextureWidth * TextureHeight;

	// Get Total Bytes of Texture - Each pixel has 4 bytes for RGBA
	TextureDataSize = TextureTotalPixels * 16;
	TextureDataSqrtSize = TextureWidth * 4;

	// Initialize Texture Data Array
	TextureDataFloat = new float[TextureTotalPixels * 4];

	// Create Dynamic Texture Object
	DynamicTexture = UTexture2D::CreateTransient(TextureWidth, TextureHeight, PF_A32B32G32R32F);
	DynamicTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	DynamicTexture->SRGB = 0;
	DynamicTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	DynamicTexture->Filter = TextureFilter::TF_Nearest;
	DynamicTexture->AddToRoot();
	DynamicTexture->UpdateResource();

	//Create Update Region Struct Instance
	TextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, TextureWidth, TextureHeight);

	//FillTexture(FLinearColor::Black);
	UpdateTexture16Bytes();
}

void AGeometryPlanet::UpdateTexture16Bytes(bool bFreeData)
{
	if (DynamicTexture == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dynamic Texture tried to Update Texture but it hasn't been initialized!"));
		return;
	}

	struct FUpdateTextureRegionsData
	{
		FTexture2DResource* Texture2DResource;
		FRHITexture2D* TextureRHI;
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


void AGeometryPlanet::SetPixelValue(int32 Offset, float X, float Y, float Z, float A)
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
