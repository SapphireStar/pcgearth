#include "NoiseApplier.h"

#include "UDynamicMesh.h"
#include "DynamicMesh/MeshNormals.h"
#include "GeometryScript/GeometryScriptSelectionTypes.h"
#include "GeometryScript/GeometryScriptTypes.h"
#include "GeometryScript/MeshDeformFunctions.h"
#include "PCG/Runtime/Planet/ShapeGenerator.h"
#include "GeometryScript/MeshDeformFunctions.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMesh/DynamicMeshAttributeSet.h"
#include "EngineDefines.h"
#include "DynamicMesh/MeshNormals.h"
#include "Async/ParallelFor.h"
#include "Spatial/SampledScalarField2.h"

#include "UDynamicMesh.h"

#define LOCTEXT_NAMESPACE "UGeometryScriptLibrary_MeshDeformFunctions"

UDynamicMesh* NoiseApplier::ApplySimpleNoise(UDynamicMesh* TargetMesh, FGeometryScriptMeshSelection Selection,
                                             UGeometryScriptDebug* Debug, UShapeGenerator* ShapeGenerator)
{
	{
		if (TargetMesh == nullptr)
		{
			UE::Geometry::AppendError(Debug, EGeometryScriptErrorType::InvalidInputs,
			                          LOCTEXT("ApplyPerlinNoiseToMesh_InvalidInput",
			                                  "ApplyPerlinNoiseToMesh: TargetMesh is Null"));
			return TargetMesh;
		}

		TargetMesh->EditMesh([&](UE::Geometry::FDynamicMesh3& EditMesh)
		{
			FVector3d Offsets[3];

			UE::Geometry::FMeshNormals Normals(&EditMesh);
			Normals.ComputeVertexNormals();

			auto GetDisplacedPosition = [&EditMesh, &Offsets, &Normals, ShapeGenerator](int32 VertexID)
			{
				FVector3d Pos = EditMesh.GetVertex(VertexID);

				float Displacement = ShapeGenerator->CalculateElevationOnPlanet(Pos);
				Pos = Displacement * Normals[VertexID];

				return Pos;
			};

			if (Selection.IsEmpty())
			{
				ParallelFor(EditMesh.MaxVertexID(), [&](int32 VertexID)
				{
					if (EditMesh.IsVertex(VertexID))
					{
						EditMesh.SetVertex(VertexID, GetDisplacedPosition(VertexID));
					}
				});
			}
			else
			{
				Selection.ProcessByVertexID(EditMesh, [&](int32 VertexID)
				{
					EditMesh.SetVertex(VertexID, GetDisplacedPosition(VertexID));
				});
			}
		}, EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown, false);

		return TargetMesh;
	}
}
