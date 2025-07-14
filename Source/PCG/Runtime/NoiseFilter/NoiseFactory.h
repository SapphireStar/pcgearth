#pragma once
#include "INoiseFilterInterface.h"
#include "PCG/Runtime/Planet/ShapeGenerator.h"

class NoiseFactory
{
public:
	static TSharedPtr<INoiseFilterInterface> CreateNoiseFilter(FNoiseLayer NoiseLayer);
};
