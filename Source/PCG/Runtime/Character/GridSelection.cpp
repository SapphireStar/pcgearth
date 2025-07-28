#include "GridSelection.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

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

	SelectedGrid = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectedGrid"));
	

	SelectedPoints.Add(CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectedPoint1")));
	SelectedPoints.Add(CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectedPoint2")));
	SelectedPoints.Add(CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectedPoint3")));
	SelectedPoints.Add(CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectedPoint4")));


	PreviewLines.Add(CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewLine1")));
	PreviewLines.Add(CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewLine2")));
	PreviewLines.Add(CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewLine3")));
	PreviewLines.Add(CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewLine4")));


}

void AGridSelectionManager::BeginPlay()
{
	Super::BeginPlay();

	SelectedGrid->SetupAttachment(RootComponent);
	SelectedGrid->SetVisibility(false);
	SelectedGrid->SetStaticMesh(SelectedGridMesh);
	SelectedGrid->SetMaterial(0, SelectedGridMaterial);
	SelectedGrid->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SelectedGrid->SetWorldScale3D(FVector(0,0,SelectedGridsThickness));
	for (auto MeshComp : SelectedPoints)
	{
		MeshComp->SetupAttachment(RootComponent);
		MeshComp->SetVisibility(false);
		MeshComp->SetStaticMesh(SelectedPointMesh);
		MeshComp->SetMaterial(0, SelectedPointMaterial);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComp->SetWorldScale3D(FVector(SelectedPointRadius, SelectedPointRadius, SelectedPointRadius));
	}
	for (auto MeshComp : PreviewLines)
	{
		MeshComp->SetupAttachment(RootComponent);
		MeshComp->SetVisibility(false);
		MeshComp->SetStaticMesh(PreviewLineMesh);
		MeshComp->SetMaterial(0, PreviewLineMaterial);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComp->SetWorldScale3D(FVector(0, PreviewLineRadius, PreviewLineRadius));
	}
	
	FBox MeshBox = GridPlaneMesh->Bounds.GetBox();
	float MeshWidth = MeshBox.Max.X - MeshBox.Min.X;
	float MeshHeight = MeshBox.Max.Y - MeshBox.Min.Y;
	float GridTotalWidth = GridWidth * GridSize;
	float GridTotalHeight = GridHeight * GridSize;
	float ScaleX = GridTotalWidth / MeshWidth;
	float ScaleY = GridTotalHeight / MeshHeight;
	GridPlaneMesh->SetWorldScale3D(FVector(ScaleX * 10, ScaleY * 10, 0.1f));
	GenerateGrid();

	TextRenderer = GetWorld()->SpawnActor<ATextRenderActor>();
	TextRenderer->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
}

void AGridSelectionManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsInGridSelectionMode)
	{
		DrawSelectedGrid();
		DrawSelectedPoints();
		DrawPreviewLines();
	}
}

void AGridSelectionManager::StartGridSelection(const FVector& StartPoint, const FRotator& Rotation)
{
	ClearSelection();

	GridRotation = Rotation;

	InitialSelectedPoint = StartPoint;
	GridCenter = InitialSelectedPoint;
	GenerateGrid();

	bIsInGridSelectionMode = true;

	SelectedGridPoints.Add(InitialSelectedPoint);

	ShowPreviewMeshes();

	UE_LOG(LogTemp, Warning, TEXT("Started grid selection at: %s with rotation: %s"),
	       *InitialSelectedPoint.ToString(), *GridRotation.ToString());
}

FBox AGridSelectionManager::EndGridSelection()
{
	bIsInGridSelectionMode = false;

	TArray<FVector> FinalShape = GetFinalGrid();

	GridBounds = FBox(FinalShape);
	
	HidePreviewMeshes();
	ClearSelection();
	return GridBounds;
}

FBox AGridSelectionManager::PeekGridSelection()
{
	TArray<FVector> FinalShape = TArray<FVector>(SelectedGridPoints);

	GridBounds = FBox(FinalShape);
    
	UE_LOG(LogTemp, Log, TEXT("PeekGridSelection: %s"), *GridBounds.ToString());
    
	return GridBounds;
}

FIntVector AGridSelectionManager::PeekGridSize()
{
	FVector LocalGridExtent = WorldToLocal(SelectedGridPoints[2]);
	int SizeX = FMath::Abs(FMath::RoundToInt(LocalGridExtent.X / GridSize));
	int SizeY = FMath::Abs(FMath::RoundToInt(LocalGridExtent.Y / GridSize));
	return FIntVector(SizeX, SizeY, 0);
}

void AGridSelectionManager::ShutDownGridSelection()
{
	bIsInGridSelectionMode = false;
	ClearSelection();
}

bool AGridSelectionManager::PreviewSelectGrid(const FVector& WorldPosition)
{
	if (!bIsInGridSelectionMode)
	{
		return false;
	}

	FVector GridPoint = SnapToGrid(WorldPosition);

	/*if (!CanSelectPoint(GridPoint))
	{
		return false;
	}*/

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

	SelectedGridPoints.Add(GridPointA);
	SelectedGridPoints.Add(GridPoint);
	SelectedGridPoints.Add(GridPointB);


	return true;
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

	FVector Extent = (GridPlaneMesh->Bounds.GetBox().Max - GridPlaneMesh->Bounds.GetBox().Min);
	FVector PlaneLocation = GridCenter -LocalToWorld( FVector(Extent.X / 2, Extent.Y/ 2, 0)) + GridCenter;
	GridPlaneMesh->SetWorldLocation(PlaneLocation);
	GridPlaneMesh->SetWorldRotation(GridRotation);
}

void AGridSelectionManager::DrawSelectedGrid()
{
	FVector LocalGridExtent = WorldToLocal(SelectedGridPoints[2]);
	if (FMath::IsNearlyZero(LocalGridExtent.X) || FMath::IsNearlyZero(LocalGridExtent.Y))
	{
		SelectedGrid->SetWorldScale3D(FVector(0, 0, SelectedGridsThickness));
		return;
	}
	FVector LocalGridMid =  LocalGridExtent * 0.5f;
	FVector WorldGridMid =  LocalToWorld(LocalGridMid);
	SelectedGrid->SetWorldLocation(WorldGridMid);
	FVector GridMeshBounds = SelectedGrid->GetStaticMesh()->GetBounds().BoxExtent;
	float ScaleX = LocalGridExtent.X / GridMeshBounds.X / 2;
	float ScaleY = LocalGridExtent.Y / GridMeshBounds.Y / 2;
	SelectedGrid->SetWorldScale3D(FVector(ScaleX, ScaleY, SelectedGridsThickness));
	SelectedGrid->SetWorldLocation(GridCenter);
	SelectedGrid->SetWorldRotation(GridRotation);
}

void AGridSelectionManager::DrawSelectedPoints()
{
	for (int32 i = 0; i < SelectedGridPoints.Num(); i++)
	{
		const FVector& Point = SelectedGridPoints[i];

		SelectedPoints[i]->SetWorldLocation(Point);
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
		FVector StartPoint = SelectedGridPoints[i] ;
		FVector EndPoint = SelectedGridPoints[i + 1] ;

		FVector Bounds = PreviewLines[i]->GetStaticMesh()->GetBounds().BoxExtent;
		float Scale = (EndPoint - StartPoint).Length() / Bounds.X / 2;
		FRotator Rotation = UKismetMathLibrary::MakeRotFromX((EndPoint - StartPoint).GetSafeNormal());
		PreviewLines[i]->SetWorldScale3D(FVector(Scale, PreviewLineRadius, PreviewLineRadius));
		PreviewLines[i]->SetWorldLocation(StartPoint);
		PreviewLines[i]->SetWorldRotation(Rotation);
	}

	if (SelectedGridPoints.Num() > 2)
	{
		FVector LastPoint = SelectedGridPoints.Last() ;
		FVector FirstPoint = InitialSelectedPoint;

		FVector Bounds = PreviewLines[PreviewLines.Num() - 1]->GetStaticMesh()->GetBounds().BoxExtent;
		float Scale = (FirstPoint - LastPoint).Length() / Bounds.X / 2;
		FRotator Rotation = UKismetMathLibrary::MakeRotFromX((FirstPoint - LastPoint).GetSafeNormal());
		PreviewLines[PreviewLines.Num()-1]->SetWorldScale3D(FVector(Scale, PreviewLineRadius, PreviewLineRadius));
		PreviewLines[PreviewLines.Num()-1]->SetWorldLocation(LastPoint);
		PreviewLines[PreviewLines.Num()-1]->SetWorldRotation(Rotation);
	}

}

void AGridSelectionManager::ShowPreviewMeshes()
{
	SelectedGrid->SetVisibility(true);
	for (auto MeshComp : SelectedPoints)
	{
		MeshComp->SetVisibility(true);
	}
	for (auto MeshComp : PreviewLines)
	{
		MeshComp->SetVisibility(true);
	}
}

void AGridSelectionManager::HidePreviewMeshes()
{
	SelectedGrid->SetVisibility(false);
	for (auto MeshComp : SelectedPoints)
	{
		MeshComp->SetVisibility(false);
	}
	for (auto MeshComp : PreviewLines)
	{
		MeshComp->SetVisibility(false);
	}
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
	int32 GridY =FMath::RoundToInt(LocalPosition.Y / GridSize);
	int32 GridZ = FMath::RoundToInt(LocalPosition.Z / GridSize);

	GridX = FMath::Clamp(GridX, -GridWidth/2, GridWidth/2);
	GridY = FMath::Clamp(GridY, -GridHeight/2, GridHeight/2);

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

void AGridSelectionManager::ClearSelection()
{
	HidePreviewMeshes();
	SelectedGridPoints.Empty();
	InitialSelectedPoint = FVector::ZeroVector;
}

TArray<FVector> AGridSelectionManager::GetFinalGrid() const
{
	TArray<FVector> FinalShape = SelectedGridPoints;

	if (FinalShape.Num() > 2 && FinalShape.Last() != InitialSelectedPoint)
	{
		FinalShape.Add(InitialSelectedPoint);
	}

	return FinalShape;
}

void AGridSelectionManager::ShowText(FVector PlayerPos)
{
	
}

void AGridSelectionManager::HideText()
{
}
