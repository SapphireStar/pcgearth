#include "SimpleNoiseFilter.h"

#include "PCG/Runtime/Utils/NoiseLibrary.h"

SimpleNoiseFilter::SimpleNoiseFilter(FNoiseLayer NoiseLayer)
{
	this->NoiseLayer = NoiseLayer;
}

float SimpleNoiseFilter::EvaluateNoise(FVector pointOnUnitSphere)
{
	if (!NoiseLayer.bEnabled) return 0.0f;
	
	auto NoiseSettings = NoiseLayer.NoiseSettings;
	float noiseValue = 0.f;
	float frequency = NoiseSettings.BaseRoughness;
	float amplitude = 1.f;
	for (int i = 0; i< NoiseSettings.NumLayers; ++i)
	{
		float v = UNoiseLibrary::Evaluate(pointOnUnitSphere * frequency + NoiseSettings.Center);
		noiseValue += (v+1)*.5f * amplitude;
		frequency *= NoiseSettings.Roughness;
		amplitude *= NoiseSettings.Persistence;
	}
	//为了保持球形，截断小于MinValue的噪声值，MinValue相当于球的半径
	noiseValue = FMath::Max(0, noiseValue - NoiseSettings.MinValue);
	return noiseValue * NoiseSettings.Strength;
}
