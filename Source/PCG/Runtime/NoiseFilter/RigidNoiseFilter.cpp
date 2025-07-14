#include "RigidNoiseFilter.h"

#include "PCG/Runtime/Utils/NoiseLibrary.h"

RigidNoiseFilter::RigidNoiseFilter(FNoiseLayer NoiseLayer)
{
	this->NoiseLayer = NoiseLayer;
}

float RigidNoiseFilter::EvaluateNoise(FVector pointOnUnitSphere)
{
	if (!NoiseLayer.bEnabled) return 0.0f;
	
	auto& NoiseSettings = NoiseLayer.NoiseSettings;
	float noiseValue = 0.f;
	float frequency = NoiseSettings.BaseRoughness;
	float amplitude = 1.f;
	float weight = 1.f;
	
	for (int i = 0; i< NoiseSettings.NumLayers; ++i)
	{
		float noisevalue = 1 - FMath::Abs(UNoiseLibrary::Evaluate(pointOnUnitSphere * frequency + NoiseSettings.Center));
		float v = noisevalue;
		v = v * v;
		v *= weight;
		//weight越来越高，意味着，更高层的Layer权重更高，产生更多细节（或者也可以相反）
		weight = FMath::Clamp(v * NoiseSettings.WeightMultiplier, 0.f, 1.f);
		noiseValue += v * amplitude;
		frequency *= NoiseSettings.Roughness;
		amplitude *= NoiseSettings.Persistence;
	}
	//为了保持球形，截断小于MinValue的噪声值，MinValue相当于球的半径
	noiseValue = FMath::Max(0, noiseValue - NoiseSettings.MinValue);
	return noiseValue * NoiseSettings.Strength;
}
