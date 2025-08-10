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
	//如果噪声的roughness很低，那么就没有细节，但如果roughness很高，又会导致没有完整的形状，因此使用噪声分层，每增加一层，噪声的roughness增加，但strength减少，也就是细节增加，但影响度减少
	for (int i = 0; i< NoiseSettings.NumLayers; ++i)
	{
		//frequency也就是roughness用于控制粗糙度，这里用pointOnUnitSphere乘以frequency代表，当frequency越大，那么使用pointOnUnitySphere在噪声图上采样时，间隔越大，间隔越大，两次采样之间的差距就越大
		float v = UNoiseLibrary::Evaluate(pointOnUnitSphere * frequency + NoiseSettings.Center);
		noiseValue += (v+1)*.5f * amplitude;
		frequency *= NoiseSettings.Roughness;
		amplitude *= NoiseSettings.Persistence;
	}
	//为了保持球形，截断小于MinValue的噪声值，MinValue相当于球的半径
	noiseValue = FMath::Max(0, noiseValue - NoiseSettings.MinValue);
	return noiseValue * NoiseSettings.Strength;
}
