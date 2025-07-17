// Fill out your copyright notice in the Description page of Project Settings.


#include "TestMineMaterialTexture.h"
#include "RHICommandList.h"
#include "Rendering/Texture2DResource.h"

// Sets default values
ATestMineMaterialTexture::ATestMineMaterialTexture()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	StaticMesh  = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);
}

// Called when the game starts or when spawned
void ATestMineMaterialTexture::BeginPlay()
{
	DynamicMaterialInstance = UMaterialInstanceDynamic::Create(Material,this);
	StaticMesh->SetMaterial(0, DynamicMaterialInstance);
	while (TextureDataSize < Positions.Num())
	{
		TextureDataSize *= 2;
	}
	InitializeTexture16Bytes();
	GenerateMineMaterialTexture();

	Super::BeginPlay();

}

// Called every frame
void ATestMineMaterialTexture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateTexture16Bytes();
	if (DynamicMaterialInstance && DynamicTexture)
		DynamicMaterialInstance->SetTextureParameterValue("SpherePos", DynamicTexture);
}

UTexture2D* ATestMineMaterialTexture::GenerateMineMaterialTexture(TArray<FVector>& Positions,
	TArray<float>& Values)
{
	if (Positions.Num() == 0 || Values.Num() == 0 || Positions.Num() != Values.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid input arrays for material texture generation"));
		return nullptr;
	}

	int32 TextureWidth = Values.Num();
	UTexture2D* DataTexture = UTexture2D::CreateTransient(TextureWidth, 1, PF_A32B32G32R32F);
    
	if (!DataTexture)
	{
		return nullptr;
	}

	// 设置纹理属性
	DataTexture->SRGB = false;
	DataTexture->CompressionSettings = TextureCompressionSettings::TC_HDR;
	DataTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	DataTexture->Filter = TextureFilter::TF_Nearest;
    
	// 获取并锁定纹理数据
	FTexture2DMipMap& Mip = DataTexture->GetPlatformData()->Mips[0];
	FColor* PixelData = static_cast<FColor*>(Mip.BulkData.Lock(LOCK_READ_WRITE));
    
	if (!PixelData)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to lock texture data"));
		return nullptr;
	}
    
	// 写入数据
	for (int32 i = 0; i < Values.Num(); i++)
	{
		PixelData[i] = FColor(
			Positions[i].X,
			Positions[i].Y,
			Positions[i].Z,
			Values[i]
		);
	}
    
	// 解锁并更新
	Mip.BulkData.Unlock();
	DataTexture->UpdateResource();
    
	return DataTexture;
	/*if (Positions.Num() == 0 || Values.Num() == 0)
	{
		Positions.Add(FVector(0.0f, 0.0f, 0.0f));
		Values.Add(0.0f);
		UE_LOG(LogTemp, Error, TEXT("Invalid number of material textures"));
	}
	UTexture2D* DataTexture = UTexture2D::CreateTransient(Values.Num(), 1, PF_A32B32G32R32F);
	DataTexture->SRGB = false;  // 重要：对于数据纹理，关闭SRGB
	DataTexture->CompressionSettings = TextureCompressionSettings::TC_HDR;
	DataTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	DataTexture->Filter = TextureFilter::TF_Nearest;  // 使用最近邻过滤，避免插值
	FLinearColor* PixelData = (FLinearColor*)DataTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    
	for(int32 i = 0; i < Values.Num(); i++)
	{
		PixelData[i] = FLinearColor(
			Positions[i].X,
			Positions[i].Y, 
			Positions[i].Z,
			Values[i]
		);
	}
    
	DataTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
	DataTexture->UpdateResource();
    
	return DataTexture;*/
}

void ATestMineMaterialTexture::GenerateMineMaterialTexture()
{
	for (int i = 0;i < Positions.Num();i++)
	{
		SetPixelValue(i, Positions[i].X, Positions[i].Y, Positions[i].Z, Values[i]);
	}
}

void ATestMineMaterialTexture::FillTexture(FLinearColor Color)
{
	for (uint32 i = 0; i < TextureTotalPixels; i++)
	{
		TextureData[i * 4] = Color.B * 255;
		TextureData[i * 4 + 1] = Color.G * 255;
		TextureData[i * 4 + 2] = Color.R * 255;
		TextureData[i * 4 + 3] = Color.A * 255;
	}
}

void ATestMineMaterialTexture::InitializeTexture()
{
	// Get Total Pixels in Texture
	TextureTotalPixels = TextureWidth * TextureHeight;
 
	// Get Total Bytes of Texture - Each pixel has 4 bytes for RGBA
	TextureDataSize = TextureTotalPixels * 4;
	TextureDataSqrtSize = TextureWidth * 4;
 
	// Initialize Texture Data Array
	TextureData = new uint8[TextureDataSize];
 
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
 
	FillTexture(FLinearColor::Black);
	UpdateTexture();
}

void ATestMineMaterialTexture::InitializeTexture16Bytes()
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

void ATestMineMaterialTexture::UpdateTexture(bool bFreeData)
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
	RegionData->SrcBpp = 4;
	RegionData->SrcData = TextureData;
 
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
			if (bFreeData) {
				FMemory::Free(RegionData->Regions);
				FMemory::Free(RegionData->SrcData);
			}
			delete RegionData;
});
}

void ATestMineMaterialTexture::UpdateTexture16Bytes(bool bFreeData)
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
			if (bFreeData) {
				FMemory::Free(RegionData->Regions);
				FMemory::Free(RegionData->SrcData);
			}
			delete RegionData;
});
}

void ATestMineMaterialTexture::SetPixelValue(int32 X, FColor Color)
{
	// If Pixel is outside of Texture return
	if (X < 0 || X >= TextureWidth) {
		return;
	}
 
	// Get the Start of the Pixel Data
	uint32 start = X * 4;
 
	// Set Pixel Value by Offsetting from the Start of the Pixel Data
	TextureData[start] = Color.B;
	TextureData[start + 1] = Color.G;
	TextureData[start + 2] = Color.R;
	TextureData[start + 3] = Color.A;
}

void ATestMineMaterialTexture::SetPixelValue(int32 Offset, float X, float Y, float Z, float A)
{
	if (Offset < 0 || Offset >= TextureWidth) {
		return;
	}
	uint32 start = Offset * 4;

	TextureDataFloat[start] = X;
	TextureDataFloat[start + 1] = Y;
	TextureDataFloat[start + 2] = Z;
	TextureDataFloat[start + 3] = A;
}

void ATestMineMaterialTexture::SetPositionAndRadius(int32 Offset, int32 X, int32 Y, int32 Z, int32 Radius)
{
	uint32 start = Offset * 4 * 4;

	
}

