#pragma once
#include "ImageUtils.h"
#include "TextureCompiler.h"

class Helper
{
public:
	static void CacheTextureSampler(UTexture2D* Texture, TArray<FLinearColor>& OutPixelColor)
	{
		if (!Texture || !Texture->IsValidLowLevel())
			return;

		Texture->WaitForStreaming();

		// 如果纹理正在编译，等待编译完成
#if WITH_EDITOR
		if (Texture->IsCompiling())
		{
			FTextureCompilingManager::Get().FinishCompilation({Texture});
		}
#endif

		// 确保平台数据可用
		if (!Texture->GetPlatformData())
		{
			UE_LOG(LogTemp, Error, TEXT("Platform data is not available"));
			return;
		}

		// 确保纹理是CPU可读的
		TextureMipGenSettings OldMipGenSettings = Texture->MipGenSettings;
		bool bOldSRGB = Texture->SRGB;

		// 临时修改设置以确保可以读取
		Texture->MipGenSettings = TMGS_NoMipmaps;
		Texture->SRGB = false;
		Texture->UpdateResource();

		// 获取平台数据
		FTexture2DMipMap* MipMap = &Texture->GetPlatformData()->Mips[0];

		// 锁定纹理数据
		FByteBulkData* RawImageData = &MipMap->BulkData;
		FColor* FormatedImageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));
		if (FormatedImageData)
		{
			int32 Width = MipMap->SizeX;
			int32 Height = MipMap->SizeY;
			OutPixelColor.Init(FColor::Black, Width * Height);
			// 读取特定像素的颜色
			for (int X = 0; X < Width; X++)
			{
				for (int Y = 0; Y < Height; Y++)
				{
					if (X < Width && Y < Height)
					{
						FColor PixelColor = FormatedImageData[Y * Width + X];
						OutPixelColor[Y * Width + X] = FLinearColor(PixelColor.R / 255.f, PixelColor.G / 255.f,
						                                            PixelColor.B / 255.f, PixelColor.A / 255.f);
						UE_LOG(LogTemp, Warning, TEXT("Read Color: %f, %f, %f, %f"), OutPixelColor[Y * Width + X].R, OutPixelColor[Y * Width + X].G, OutPixelColor[Y * Width + X].B, OutPixelColor[Y * Width + X].A);
					}
				}
			}

			// 解锁数据
			RawImageData->Unlock();
		}

		// 恢复原始设置
		Texture->MipGenSettings = OldMipGenSettings;
		Texture->SRGB = bOldSRGB;
		Texture->UpdateResource();
	}

	static FLinearColor ReadTexturePixels(TArray<FLinearColor>& PixelColors, int x, int y, int width, int height)
	{
		if (PixelColors.IsEmpty())
		{
			return FLinearColor::Black;
		}
		x = FMath::Clamp(x, 0, width - 1);
		y = FMath::Clamp(y, 0, height - 1);
		return PixelColors[y * width + x];
	}

	static FLinearColor GetPixelAsFloat(TArray<FLinearColor>& OutPixelColor, TObjectPtr<UTexture2D> Texture, float x,
	                                    float y, int width, int height)
	{
		/*if (!Texture) return FLinearColor::Black;
		TextureCompressionSettings OldCompressionSettings = Texture->CompressionSettings; TextureMipGenSettings OldMipGenSettings = Texture->MipGenSettings; bool OldSRGB = Texture->SRGB;

		Texture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
		Texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
		Texture->SRGB = false;
		Texture->UpdateResource();


			const FColor* FormatedImageData = static_cast<const FColor*>( Texture->GetPlatformData()->Mips[0].BulkData.LockReadOnly());

		int X = x * Texture->GetSizeX();
		int Y = y * Texture->GetSizeY();
		FColor PixelColor = FormatedImageData[Y * Texture->GetSizeX() + X];
		
		Texture->GetPlatformData()->Mips[0].BulkData.Unlock();

		Texture->CompressionSettings = OldCompressionSettings;
		Texture->MipGenSettings = OldMipGenSettings;
		Texture->SRGB = OldSRGB;
		Texture->UpdateResource();

		return FLinearColor(PixelColor.R / 255.f,  PixelColor.G / 255.f, PixelColor.B / 255.f, PixelColor.A / 255.f);*/
		return FLinearColor::Black;
	}

	static FLinearColor BilinearSample(TObjectPtr<UTexture2D> texture, TArray<FLinearColor>& PixelColors, float x,
	                                   float y)
	{
		float width = texture->GetSizeX() - 1;
		float height = texture->GetSizeY() - 1;

		int x1 = FMath::Floor(x * width);
		int y1 = FMath::Floor(y * height);
		int x2 = FMath::Clamp(x1 + 1, 0, width);
		int y2 = FMath::Clamp(y1 + 1, 0, height);

		float xp = x * width - x1;
		float yp = y * height - y1;

		FLinearColor p11 = ReadTexturePixels(PixelColors, x1, y1, width, height);
		FLinearColor p21 = ReadTexturePixels(PixelColors, x2, y1, width, height);
		FLinearColor p12 = ReadTexturePixels(PixelColors, x1, y2, width, height);
		FLinearColor p22 = ReadTexturePixels(PixelColors, x2, y2, width, height);

		FLinearColor px1 = FMath::Lerp(p11, p21, xp);
		FLinearColor px2 = FMath::Lerp(p12, p22, xp);

		return FMath::Lerp(px1, px2, yp);
	}
};
