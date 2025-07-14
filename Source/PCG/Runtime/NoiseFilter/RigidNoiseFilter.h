#pragma once
#include "INoiseFilterInterface.h"
#include "PCG/Runtime/Planet/ShapeGenerator.h"

class RigidNoiseFilter : public INoiseFilterInterface
{
public:
	RigidNoiseFilter() = delete;
	RigidNoiseFilter(FNoiseLayer NoiseLayer);
	virtual float EvaluateNoise(FVector pointOnUnitSphere) override;
	
private:
	FNoiseLayer NoiseLayer;
};
