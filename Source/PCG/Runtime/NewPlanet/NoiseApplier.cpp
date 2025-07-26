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
#include "TerrainDataTypes.h"
#include "DynamicMesh/MeshNormals.h"
#include "Async/ParallelFor.h"
#include "Spatial/SampledScalarField2.h"

#include "UDynamicMesh.h"
#include "GeometryScript/MeshQueryFunctions.h"

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

float NoiseApplier::CraterEffect(FVector Position, FVector CraterCenter, float CraterRadius, float CraterDepth, float CraterRimHeight)
{
	float Distance = FVector::Dist(Position, CraterCenter);
	float NormalizedDist = Distance / CraterRadius;
    
	if (NormalizedDist > 1.0f) return 0.0f;
    
	float Depression = -pow(1.0f - NormalizedDist, 2.0f) * CraterDepth;
    
	float RimHeight = 0.0f;
	if (NormalizedDist > 0.8f && NormalizedDist < 1.0f)
	{
		float RimFactor = sin((NormalizedDist - 0.8f) * PI / 0.2f);
		RimHeight = RimFactor * CraterRimHeight;
	}
	return Depression + RimHeight;
}

FVector NoiseApplier::ApplyCraterEffect(UDynamicMesh* TargetMesh, int32 VertexID, FVector ActorPosition, FCraterData CraterData)
{
	bool bIsValidVertex = false;
	FVector VertexPosition = UGeometryScriptLibrary_MeshQueryFunctions::GetVertexPosition(TargetMesh, VertexID, bIsValidVertex);
	if (!bIsValidVertex)
	{
		UE_LOG(LogTemp, Error, TEXT("ApplyCraterEffect: Vertex is not valid!"));
		return FVector::ZeroVector;
	}
    
	float CraterContribution = 0.0f;
	
	CraterContribution += CraterEffect(VertexPosition, CraterData.CraterCenter - ActorPosition, CraterData.CraterRadius, CraterData.CraterDepth, CraterData.CraterRimHeight);
    
	FVector Normal = VertexPosition.GetSafeNormal();

	return VertexPosition + Normal * CraterContribution;
}
