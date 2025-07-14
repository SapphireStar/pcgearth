#pragma once
#include "ShapeGenerator.h"

class UColorGenerator
{
public:
	UColorGenerator() = delete;
	UColorGenerator(FColorSettings Settings);
	void UpdateElevation(TSharedPtr<MinMax> elevationMinMax, UMaterialInstanceDynamic* MaterialInstanceDynamic);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> texture;
private:
	const int TextureResolution = 50;
	FColorSettings ColorSettings;
};
