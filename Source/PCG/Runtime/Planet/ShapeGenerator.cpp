// Fill out your copyright notice in the Description page of Project Settings.


#include "ShapeGenerator.h"

#include "PCG/Runtime/NoiseFilter/NoiseFactory.h"
#include "PCG/Runtime/Utils/NoiseLibrary.h"

FShapeSettings::FShapeSettings()
{
}

FShapeSettings::FShapeSettings(const FShapeSettings& Other)
{
	PlanetRadius = Other.PlanetRadius;
	for (int i = 0; i< Other.NoiseLayers.Num(); i++)
	{
		NoiseLayers.Add(Other.NoiseLayers[i]);
	}
}


void UShapeGenerator::Initialize(FShapeSettings ShapeSettings)
{
	this->ShapeSettings = ShapeSettings;
	for (int i = 0; i < ShapeSettings.NoiseLayers.Num(); i++)
	{
		NoiseFilters.Add(NoiseFactory::CreateNoiseFilter(ShapeSettings.NoiseLayers[i]));
	}

	ElevationMinMax =MakeShared<MinMax>();
}

FVector UShapeGenerator::CalculatePointOnPlanet(FVector pointOnUnitSphere)
{
	float firstLayerValue = 0.f;
	float elevation = 0.f;
	if (ShapeSettings.NoiseLayers.Num() > 0)
	{
		firstLayerValue = NoiseFilters[0]->EvaluateNoise(pointOnUnitSphere);
		if (ShapeSettings.NoiseLayers[0].bEnabled)
			elevation = firstLayerValue;
	}
	for (int i = 1; i< ShapeSettings.NoiseLayers.Num(); i++)
	{
		if (!ShapeSettings.NoiseLayers[i].bEnabled)
			continue;
		float mask = (ShapeSettings.NoiseLayers[i].bUseFirstLayerAsMask)? firstLayerValue : 1;
		elevation += NoiseFilters[i]->EvaluateNoise(pointOnUnitSphere) * mask;	
	}
	elevation = ShapeSettings.PlanetRadius * (1 + elevation);
	ElevationMinMax->AddValue(elevation);
	return pointOnUnitSphere * elevation;
}

float UShapeGenerator::CalculateElevationOnPlanet(FVector pointOnUnitSphere)
{
	float firstLayerValue = 0.f;
	float elevation = 0.f;
	if (ShapeSettings.NoiseLayers.Num() > 0)
	{
		firstLayerValue = NoiseFilters[0]->EvaluateNoise(pointOnUnitSphere);
		if (ShapeSettings.NoiseLayers[0].bEnabled)
			elevation = firstLayerValue;
	}
	for (int i = 1; i< ShapeSettings.NoiseLayers.Num(); i++)
	{
		if (!ShapeSettings.NoiseLayers[i].bEnabled)
			continue;
		float mask = (ShapeSettings.NoiseLayers[i].bUseFirstLayerAsMask)? firstLayerValue : 1;
		elevation += NoiseFilters[i]->EvaluateNoise(pointOnUnitSphere) * mask;	
	}
	elevation = ShapeSettings.PlanetRadius * (1 + elevation);
	ElevationMinMax->AddValue(elevation);
	return elevation;
}
