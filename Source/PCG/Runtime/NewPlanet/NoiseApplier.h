#pragma once

struct FShapeSettings;
class UShapeGenerator;
class UDynamicMesh;
struct FGeometryScriptMeshSelection;
struct FGeometryScriptPerlinNoiseOptions;
class UGeometryScriptDebug;

class NoiseApplier
{
public:
	static UDynamicMesh* ApplySimpleNoise(
	UDynamicMesh* TargetMesh,
	FGeometryScriptMeshSelection Selection,
	UGeometryScriptDebug* Debug,
	UShapeGenerator* ShapeGenerator);
};
