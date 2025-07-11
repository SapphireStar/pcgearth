// Fill out your copyright notice in the Description page of Project Settings.


#include "ShapeGenerator.h"

#include "PCG/Runtime/Utils/NoiseLibrary.h"

void UShapeGenerator::Initialize(FShapeSettings ShapeSettings)
{
	this->ShapeSettings = ShapeSettings;
}

FVector UShapeGenerator::CalculatePointOnPlanet(FVector pointOnUnitSphere)
{
	float elevation =  UNoiseLibrary::Evaluate(pointOnUnitSphere);
	return pointOnUnitSphere * ShapeSettings.PlanetRadius * elevation;
}
