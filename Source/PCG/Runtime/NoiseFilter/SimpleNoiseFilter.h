#pragma once
#include "INoiseFilterInterface.h"
#include "PCG/Runtime/Planet/ShapeGenerator.h"

class SimpleNoiseFilter : public INoiseFilterInterface
{
public:
	SimpleNoiseFilter() = delete;
	SimpleNoiseFilter(FNoiseLayer NoiseLayer);
	virtual float EvaluateNoise(FVector pointOnUnitSphere) override;

private:
	FNoiseLayer NoiseLayer;
};
