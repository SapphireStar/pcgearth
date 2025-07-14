#pragma once

class INoiseFilterInterface
{
public:
	virtual ~INoiseFilterInterface() {}
	virtual float EvaluateNoise(FVector pointOnUnitSphere) = 0;
};
