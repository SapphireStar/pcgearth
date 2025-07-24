#include "GridSelection.h"

#include "EngineUtils.h"
#include "PCG/Runtime/NewWFC/WFCGeneratorComponent.h"

AGridSelectionManager::AGridSelectionManager()
{
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;

	//创建网格平面（设置为不可见，仅用于碰撞检测）
	ECollisionChannel CollisionChannel;
	FCollisionResponseParams ResponseParams;

	GridPlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GridPlaneMesh"));
	GridPlaneMesh->SetupAttachment(RootComponent);
	GridPlaneMesh->SetVisibility(false);
	GridPlaneMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AGridSelectionManager::BeginPlay()
{
	Super::BeginPlay();
	FBox MeshBox = GridPlaneMesh->Bounds.GetBox();
	float MeshWidth = MeshBox.Max.X - MeshBox.Min.X;
	float MeshHeight = MeshBox.Max.Y - MeshBox.Min.Y;
	float GridTotalWidth = GridWidth * GridSize;
	float GridTotalHeight = GridHeight * GridSize;
	float ScaleX = GridTotalWidth / MeshWidth;
	float ScaleY = GridTotalHeight / MeshHeight;
	GridPlaneMesh->SetWorldScale3D(FVector(ScaleX, ScaleY, 0.1f));
	GenerateGrid();
}

void AGridSelectionManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsInGridSelectionMode)
	{
		DrawDebugGrid();
		DrawSelectedPoints();
		DrawPreviewLines();
	}
}

void AGridSelectionManager::StartGridSelection(const FVector& StartPoint, const FRotator& Rotation)
{
	ClearSelection();

	GridRotation = Rotation;
	
	InitialSelectedPoint = SnapToGrid(StartPoint);
	
	GridCenter = InitialSelectedPoint;
	GenerateGrid();

	bIsInGridSelectionMode = true;

	SelectedGridPoints.Add(InitialSelectedPoint);

	UE_LOG(LogTemp, Warning, TEXT("Started grid selection at: %s with rotation: %s"), 
		*InitialSelectedPoint.ToString(), *GridRotation.ToString());
}

FBox AGridSelectionManager::EndGridSelection()
{
	bIsInGridSelectionMode = false;

	TArray<FVector> FinalShape = GetFinalShape();

	DrawFinalShape(FinalShape);

	GridBounds.Min = FVector::OneVector * 10000000;
	GridBounds.Max = -FVector::OneVector * 10000000;
	for (int i = 0; i < FinalShape.Num(); i++)
	{
		if (FinalShape[i].X < GridBounds.Min.X)
		{
			GridBounds.Min.X = FinalShape[i].X;
		}
		if (FinalShape[i].X > GridBounds.Max.X)
		{
			GridBounds.Max.X = FinalShape[i].X;
		}
		if (FinalShape[i].Y < GridBounds.Min.Y)
		{
			GridBounds.Min.Y = FinalShape[i].Y;
		}
		if (FinalShape[i].Y > GridBounds.Max.Y)
		{
			GridBounds.Max.Y = FinalShape[i].Y;
		}
		if (FinalShape[i].Z < GridBounds.Min.Z)
		{
			GridBounds.Min.Z = FinalShape[i].Z;
		}
		if (FinalShape[i].Z > GridBounds.Max.Z)
		{
			GridBounds.Max.Z = FinalShape[i].Z;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Grid selection completed with %d points"), FinalShape.Num());

	ClearSelection();
	return GridBounds;
}

FBox AGridSelectionManager::PeekGridSelection()
{
	TArray<FVector> FinalShape = SelectedGridPoints;

	if (FinalShape.Num() > 2 && FinalShape.Last() != InitialSelectedPoint)
	{
		FinalShape.Add(InitialSelectedPoint);
	}

	GridBounds.Min = FVector::OneVector * 10000000;
	GridBounds.Max = -FVector::OneVector * 10000000;
	for (int i = 0; i < FinalShape.Num(); i++)
	{
		if (FinalShape[i].X < GridBounds.Min.X)
		{
			GridBounds.Min.X = FinalShape[i].X;
		}
		if (FinalShape[i].X > GridBounds.Max.X)
		{
			GridBounds.Max.X = FinalShape[i].X;
		}
		if (FinalShape[i].Y < GridBounds.Min.Y)
		{
			GridBounds.Min.Y = FinalShape[i].Y;
		}
		if (FinalShape[i].Y > GridBounds.Max.Y)
		{
			GridBounds.Max.Y = FinalShape[i].Y;
		}
		if (FinalShape[i].Z < GridBounds.Min.Z)
		{
			GridBounds.Min.Z = FinalShape[i].Z;
		}
		if (FinalShape[i].Z > GridBounds.Max.Z)
		{
			GridBounds.Max.Z = FinalShape[i].Z;
		}
	}
	
	return GridBounds;
}

void AGridSelectionManager::ShutDownGridSelection()
{
	bIsInGridSelectionMode = false;
	ClearSelection();
}

bool AGridSelectionManager::TrySelectGridPoint(const FVector& WorldPosition)
{
	if (!bIsInGridSelectionMode)
	{
		return false;
	}

	FVector GridPoint = SnapToGrid(WorldPosition);

	if (!CanSelectPoint(GridPoint))
	{
		DrawDebugSphere(GetWorld(), GridPoint + FVector(0, 0, 20), 30.0f, 12, FColor::Red, false, 1.0f);
		DrawDebugString(GetWorld(), GridPoint + FVector(0, 0, 50), TEXT("Cannot Select!"), nullptr, FColor::Red, 1.0f);
		return false;
	}

	if (WouldCreateCrossingEdge(GridPoint))
	{
		DrawDebugSphere(GetWorld(), GridPoint + FVector(0, 0, 20), 30.0f, 12, FColor::Orange, false, 1.0f);
		DrawDebugString(GetWorld(), GridPoint + FVector(0, 0, 50), TEXT("Crossing Edge!"), nullptr, FColor::Orange,
		                1.0f);
		UE_LOG(LogTemp, Warning, TEXT("Cannot select point - would create crossing edge"));
		return false;
	}

	SelectedGridPoints.Add(GridPoint);

	DrawDebugSphere(GetWorld(), GridPoint + FVector(0, 0, 20), 25.0f, 12, FColor::Green, false, 5.f);
	DrawDebugString(GetWorld(), GridPoint + FVector(0, 0, 50), TEXT("Selected!"), nullptr, FColor::Green, 5.f);

	UE_LOG(LogTemp, Log, TEXT("Selected grid point: %s"), *GridPoint.ToString());
	return true;
}

bool AGridSelectionManager::PreviewSelectGrid(const FVector& WorldPosition)
{
	if (!bIsInGridSelectionMode)
	{
		return false;
	}

	FVector GridPoint = SnapToGrid(WorldPosition);

	if (!CanSelectPoint(GridPoint))
	{
		return false;
	}

	while (SelectedGridPoints.Num() > 1)
	{
		SelectedGridPoints.RemoveAt(1);
	}

	FVector LocalInitial = WorldToLocal(SelectedGridPoints[0]);
	FVector LocalTarget = WorldToLocal(GridPoint);
	
	FVector LocalPointA = FVector(LocalInitial.X, LocalTarget.Y, LocalTarget.Z);
	FVector LocalPointB = FVector(LocalTarget.X, LocalInitial.Y, LocalTarget.Z);
	
	FVector GridPointA = LocalToWorld(LocalPointA);
	FVector GridPointB = LocalToWorld(LocalPointB);
	
	SelectedGridPoints.Add(SnapToGrid(GridPointA));
	SelectedGridPoints.Add(GridPoint);
	SelectedGridPoints.Add(SnapToGrid(GridPointB));

	return true;
}

bool AGridSelectionManager::CanSelectPoint(const FVector& GridPoint) const
{
	if (!IsValidGridPosition(GridPoint))
	{
		return false;
	}

	if (SelectedGridPoints.Contains(GridPoint))
	{
		return false;
	}

	const bool* bAvailable = GridPointAvailability.Find(GridPoint);
	return bAvailable && *bAvailable;
}

void AGridSelectionManager::GenerateGrid()
{
	GridPoints.Empty();
	GridPointAvailability.Empty();

	for (int32 X = -GridWidth / 2; X <= GridWidth / 2; X++)
	{
		for (int32 Y = -GridHeight / 2; Y <= GridHeight / 2; Y++)
		{
			FVector LocalGridPoint = FVector(X * GridSize, Y * GridSize, 0);
			FVector WorldGridPoint = LocalToWorld(LocalGridPoint);
			
			GridPoints.Add(WorldGridPoint);
			GridPointAvailability.Add(WorldGridPoint, true);
		}
	}
	
	GridPlaneMesh->UpdateCollisionFromStaticMesh();
	
	FVector PlaneLocation = GridCenter - LocalToWorld(FVector(GridWidth * GridSize / 2, GridHeight * GridSize / 2, 0)) + GridCenter;
	GridPlaneMesh->SetWorldLocation(PlaneLocation);
	GridPlaneMesh->SetWorldRotation(GridRotation);
}

void AGridSelectionManager::DrawDebugGrid()
{
	if (!GetWorld())
	{
		return;
	}

	FColor GridColor = FColor::White;
	float GridAlpha = 0.3f;
	GridColor.A = (uint8)(255 * GridAlpha);

	for (int32 Y = -GridHeight / 2; Y <= GridHeight / 2; Y++)
	{
		FVector LocalStart = FVector(-GridWidth / 2 * GridSize, Y * GridSize, 0);
		FVector LocalEnd = FVector(GridWidth / 2 * GridSize, Y * GridSize, 0);
		
		FVector StartPoint = LocalToWorld(LocalStart);
		FVector EndPoint = LocalToWorld(LocalEnd);
		
		DrawDebugLine(GetWorld(), StartPoint, EndPoint, GridColor, false, 0.0f, 0, 1.0f);
	}

	for (int32 X = -GridWidth / 2; X <= GridWidth / 2; X++)
	{
		FVector LocalStart = FVector(X * GridSize, -GridHeight / 2 * GridSize, 0);
		FVector LocalEnd = FVector(X * GridSize, GridHeight / 2 * GridSize, 0);
		
		FVector StartPoint = LocalToWorld(LocalStart);
		FVector EndPoint = LocalToWorld(LocalEnd);
		
		DrawDebugLine(GetWorld(), StartPoint, EndPoint, GridColor, false, 0.0f, 0, 1.0f);
	}

	for (const FVector& GridPoint : GridPoints)
	{
		if (IsValidGridPosition(GridPoint))
		{
			FColor PointColor = FColor::Cyan;
			if (SelectedGridPoints.Contains(GridPoint))
			{
				PointColor = FColor::Yellow;
			}
			else if (GridPoint == InitialSelectedPoint)
			{
				PointColor = FColor::Green;
			}

			DrawDebugSphere(GetWorld(), GridPoint + FVector(0, 0, 5), 5.0f, 8, PointColor, false, 0.0f);
		}
	}

	FVector LocalMin = FVector(-GridWidth / 2 * GridSize, -GridHeight / 2 * GridSize, -10.0f);
	FVector LocalMax = FVector(GridWidth / 2 * GridSize, GridHeight / 2 * GridSize, 10.0f);
	FVector LocalCenter = (LocalMin + LocalMax) * 0.5f;
	FVector LocalExtent = (LocalMax - LocalMin) * 0.5f;
	
	FVector WorldCenter = LocalToWorld(LocalCenter);
	
	DrawDebugBox(GetWorld(), WorldCenter, LocalExtent, GridRotation.Quaternion(), FColor::Blue, false, 0.0f, 0, 2.0f);
}

void AGridSelectionManager::DrawSelectedPoints()
{
	if (!GetWorld())
	{
		return;
	}

	for (int32 i = 0; i < SelectedGridPoints.Num(); i++)
	{
		const FVector& Point = SelectedGridPoints[i];

		FColor PointColor = (Point == InitialSelectedPoint) ? FColor::Green : FColor::Red;
		DrawDebugSphere(GetWorld(), Point + FVector(0, 0, 15), 15.0f, 12, PointColor, false, 0.0f, 0, 2.0f);

		FString IndexText = FString::Printf(TEXT("%d"), i);
		DrawDebugString(GetWorld(), Point + FVector(0, 0, 35), IndexText, nullptr, FColor::White, 0.0f);

		FString CoordText = FString::Printf(TEXT("(%.0f, %.0f)"), Point.X, Point.Y);
		DrawDebugString(GetWorld(), Point + FVector(0, 0, 50), CoordText, nullptr, FColor::Cyan, 0.0f);
	}
}

void AGridSelectionManager::DrawPreviewLines()
{
	if (!GetWorld() || SelectedGridPoints.Num() < 2)
	{
		return;
	}

	for (int32 i = 0; i < SelectedGridPoints.Num() - 1; i++)
	{
		FVector StartPoint = SelectedGridPoints[i] + FVector(0, 0, 10);
		FVector EndPoint = SelectedGridPoints[i + 1] + FVector(0, 0, 10);
		DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Yellow, false, 0.0f, 0, 3.0f);

		FVector MidPoint = (StartPoint + EndPoint) * 0.5f;
		FVector Direction = (EndPoint - StartPoint).GetSafeNormal();
		DrawDebugDirectionalArrow(GetWorld(), MidPoint - Direction * 25.0f, MidPoint + Direction * 25.0f,
		                          20.0f, FColor::Orange, false, 0.0f, 0, 2.0f);
	}

	if (SelectedGridPoints.Num() > 2)
	{
		FVector LastPoint = SelectedGridPoints.Last() + FVector(0, 0, 10);
		FVector FirstPoint = InitialSelectedPoint + FVector(0, 0, 10);

		FVector Direction = (FirstPoint - LastPoint).GetSafeNormal();
		float Distance = FVector::Dist(LastPoint, FirstPoint);
		float DashLength = 20.0f;

		for (float t = 0; t < Distance; t += DashLength * 2)
		{
			FVector DashStart = LastPoint + Direction * t;
			FVector DashEnd = LastPoint + Direction * FMath::Min(t + DashLength, Distance);
			DrawDebugLine(GetWorld(), DashStart, DashEnd, FColor::Magenta, false, 0.0f, 0, 2.0f);
		}

		DrawDebugDirectionalArrow(GetWorld(), LastPoint, FirstPoint, 30.0f, FColor::Purple, false, 0.0f, 0, 2.0f);
	}
}

void AGridSelectionManager::DrawFinalShape(const TArray<FVector>& Shape)
{
	if (!GetWorld() || Shape.Num() < 3)
	{
		return;
	}

	for (int32 i = 0; i < Shape.Num(); i++)
	{
		FVector StartPoint = Shape[i] + FVector(0, 0, 20);
		FVector EndPoint = Shape[(i + 1) % Shape.Num()] + FVector(0, 0, 20);
		DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Emerald, false, 5.0f, 0, 5.0f);
	}

	FVector Centroid = FVector::ZeroVector;
	for (const FVector& Point : Shape)
	{
		Centroid += Point;
	}
	Centroid /= Shape.Num();

	DrawDebugSphere(GetWorld(), Centroid + FVector(0, 0, 25), 20.0f, 16, FColor::Yellow, false, 5.0f, 0, 3.0f);
	DrawDebugString(GetWorld(), Centroid + FVector(0, 0, 50), TEXT("Shape Center"), nullptr, FColor::Yellow, 5.0f);

	FString ShapeInfo = FString::Printf(TEXT("Final Shape: %d vertices"), Shape.Num());
	DrawDebugString(GetWorld(), Centroid + FVector(0, 0, 70), ShapeInfo, nullptr, FColor::White, 5.0f);
}

void AGridSelectionManager::UpdatePreviewLines()
{
}

void AGridSelectionManager::CreatePointMarker(const FVector& Position)
{
}

void AGridSelectionManager::CreatePreviewLine(const FVector& Start, const FVector& End)
{
}

void AGridSelectionManager::ClearVisualElements()
{

}

FVector AGridSelectionManager::SnapToGrid(const FVector& WorldPosition) const
{
	FVector LocalPosition = WorldToLocal(WorldPosition);
	
	FVector SnappedLocal = SnapToLocalGrid(LocalPosition);
	
	return LocalToWorld(SnappedLocal);
}

FVector AGridSelectionManager::SnapToLocalGrid(const FVector& LocalPosition) const
{
	int32 GridX = FMath::RoundToInt(LocalPosition.X / GridSize);
	int32 GridY = FMath::RoundToInt(LocalPosition.Y / GridSize);
	int32 GridZ = FMath::RoundToInt(LocalPosition.Z / GridSize);

	return FVector(GridX * GridSize, GridY * GridSize, GridZ * GridSize);
}

FVector AGridSelectionManager::WorldToLocal(const FVector& WorldPosition) const
{
	FVector RelativePos = WorldPosition - GridCenter;
	
	FQuat InverseRotation = GridRotation.Quaternion().Inverse();
	return InverseRotation.RotateVector(RelativePos);
}

FVector AGridSelectionManager::LocalToWorld(const FVector& LocalPosition) const
{
	FQuat Rotation = GridRotation.Quaternion();
	FVector RotatedPos = Rotation.RotateVector(LocalPosition);
	
	return RotatedPos + GridCenter;
}

bool AGridSelectionManager::IsValidGridPosition(const FVector& GridPosition) const
{
	FVector LocalPos = WorldToLocal(GridPosition);
	int32 GridX = FMath::RoundToInt(LocalPos.X / GridSize);
	int32 GridY = FMath::RoundToInt(LocalPos.Y / GridSize);

	return FMath::Abs(GridX) <= GridWidth / 2 && FMath::Abs(GridY) <= GridHeight / 2;
}

bool AGridSelectionManager::WouldCreateCrossingEdge(const FVector& NewPoint) const
{
	if (SelectedGridPoints.Num() < 2)
	{
		return false;
	}

	FVector LastPoint = SelectedGridPoints.Last();

	for (int32 i = 0; i < SelectedGridPoints.Num() - 1; i++)
	{
		if (DoLinesIntersect(LastPoint, NewPoint, SelectedGridPoints[i], SelectedGridPoints[i + 1]))
		{
			FVector IntersectionPoint = GetLineIntersection(LastPoint, NewPoint, SelectedGridPoints[i],
			                                                SelectedGridPoints[i + 1]);
			DrawDebugSphere(GetWorld(), IntersectionPoint + FVector(0, 0, 30), 20.0f, 12, FColor::Red, false, 2.0f);
			DrawDebugString(GetWorld(), IntersectionPoint + FVector(0, 0, 60), TEXT("CROSSING!"), nullptr, FColor::Red,
			                2.0f);
			return true;
		}
	}

	if (SelectedGridPoints.Num() > 2)
	{
		for (int32 i = 1; i < SelectedGridPoints.Num() - 1; i++)
		{
			if (DoLinesIntersect(NewPoint, InitialSelectedPoint, SelectedGridPoints[i], SelectedGridPoints[i + 1]))
			{
				return true;
			}
		}
	}

	return false;
}

bool AGridSelectionManager::DoLinesIntersect(const FVector& Line1Start, const FVector& Line1End,
                                             const FVector& Line2Start, const FVector& Line2End) const
{
	auto CrossProduct = [](const FVector2D& A, const FVector2D& B) -> float
	{
		return A.X * B.Y - A.Y * B.X;
	};

	FVector2D P1(Line1Start.X, Line1Start.Y);
	FVector2D P2(Line1End.X, Line1End.Y);
	FVector2D P3(Line2Start.X, Line2Start.Y);
	FVector2D P4(Line2End.X, Line2End.Y);

	FVector2D D1 = P2 - P1;
	FVector2D D2 = P4 - P3;

	float CrossD1D2 = CrossProduct(D1, D2);

	if (FMath::Abs(CrossD1D2) < SMALL_NUMBER)
	{
		return false;
	}

	FVector2D P3P1 = P1 - P3;
	float t = CrossProduct(P3P1, D2) / CrossD1D2;
	float u = CrossProduct(P3P1, D1) / CrossD1D2;

	return (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f);
}

FVector AGridSelectionManager::GetLineIntersection(const FVector& Line1Start, const FVector& Line1End,
                                                   const FVector& Line2Start, const FVector& Line2End) const
{
	FVector2D P1(Line1Start.X, Line1Start.Y);
	FVector2D P2(Line1End.X, Line1End.Y);
	FVector2D P3(Line2Start.X, Line2Start.Y);
	FVector2D P4(Line2End.X, Line2End.Y);

	FVector2D D1 = P2 - P1;
	FVector2D D2 = P4 - P3;

	float CrossD1D2 = D1.X * D2.Y - D1.Y * D2.X;

	if (FMath::Abs(CrossD1D2) < SMALL_NUMBER)
	{
		return Line1Start; 
	}

	FVector2D P3P1 = P1 - P3;
	float t = (P3P1.X * D2.Y - P3P1.Y * D2.X) / CrossD1D2;

	FVector2D IntersectionPoint = P1 + D1 * t;
	return FVector(IntersectionPoint.X, IntersectionPoint.Y, Line1Start.Z);
}

void AGridSelectionManager::ClearSelection()
{
	SelectedGridPoints.Empty();
	InitialSelectedPoint = FVector::ZeroVector;
}

TArray<FVector> AGridSelectionManager::GetFinalShape() const
{
	TArray<FVector> FinalShape = SelectedGridPoints;

	if (FinalShape.Num() > 2 && FinalShape.Last() != InitialSelectedPoint)
	{
		FinalShape.Add(InitialSelectedPoint);
	}

	return FinalShape;
}