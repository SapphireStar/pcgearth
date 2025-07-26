#pragma once
#include "TerrainDataTypes.h"

struct FShapeSettings;
class UShapeGenerator;
class UDynamicMesh;
struct FGeometryScriptMeshSelection;
struct FGeometryScriptPerlinNoiseOptions;
class UGeometryScriptDebug;

class PCG_API NoiseApplier
{
public:
	static UDynamicMesh* ApplySimpleNoise(
	UDynamicMesh* TargetMesh,
	FGeometryScriptMeshSelection Selection,
	UGeometryScriptDebug* Debug,
	UShapeGenerator* ShapeGenerator);

	static float CraterEffect(FVector Position, FVector CraterCenter, float CraterRadius, float CraterDepth, float CraterRimHeight);
	static FVector ApplyCraterEffect(UDynamicMesh* TargetMesh, int32 VertexID, FVector ActorPosition, FCraterData CraterData);
};
