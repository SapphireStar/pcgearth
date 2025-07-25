// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridSelection.generated.h"

UCLASS()
class PCG_API AGridSelectionManager : public AActor
{
   GENERATED_BODY()

public:
    AGridSelectionManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float GridSize = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    int32 GridWidth = 20;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    int32 GridHeight = 20;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    FVector GridCenter = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    FRotator GridRotation = FRotator::ZeroRotator;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* GridPlaneMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* GridMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* SelectedPointMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* PreviewLineMaterial;

    UPROPERTY(BlueprintReadOnly, Category = "Selection State")
    bool bIsInGridSelectionMode = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Selection State")
    FVector InitialSelectedPoint;
    
    UPROPERTY(BlueprintReadOnly, Category = "Selection State")
    TArray<FVector> SelectedGridPoints;
    
    UPROPERTY(BlueprintReadOnly, Category = "Selection State")
    TArray<UStaticMeshComponent*> PointMarkers;
    
    UPROPERTY(BlueprintReadOnly, Category = "Selection State")
    TArray<UStaticMeshComponent*> PreviewLines;

public:
    UFUNCTION(BlueprintCallable, Category = "Grid Selection")
    void StartGridSelection(const FVector& StartPoint, const FRotator& Rotation = FRotator::ZeroRotator);
    
    UFUNCTION(BlueprintCallable, Category = "Grid Selection")
    FBox EndGridSelection();

    UFUNCTION(BlueprintCallable, Category = "Grid Selection")
    FBox PeekGridSelection();

    UFUNCTION(BlueprintCallable, Category = "Grid Selection")
    void ShutDownGridSelection();

    UFUNCTION(BlueprintCallable, Category = "Grid Selection")
    FRotator GetGridRotation() const{return GridRotation;}

    UFUNCTION(BlueprintCallable, Category = "Grid Selection")
    float GetGridSize() const {return  GridSize;}
    
    UFUNCTION(BlueprintCallable, Category = "Grid Selection")
    bool TrySelectGridPoint(const FVector& WorldPosition);

    UFUNCTION(BlueprintCallable, Category = "Grid Selection")
    bool PreviewSelectGrid(const FVector& WorldPosition);
    
    UFUNCTION(BlueprintCallable, Category = "Grid Selection")
    bool CanSelectPoint(const FVector& GridPoint) const;
    
    UFUNCTION(BlueprintCallable, Category = "Grid Selection")
    void ClearSelection();
    
    UFUNCTION(BlueprintCallable, Category = "Grid Selection")
    TArray<FVector> GetFinalShape() const;

protected:
    void GenerateGrid();
    void UpdatePreviewLines();
    void CreatePointMarker(const FVector& Position);
    void CreatePreviewLine(const FVector& Start, const FVector& End);
    void ClearVisualElements();

    void DrawDebugGrid();
    void DrawSelectedPoints();
    void DrawPreviewLines();
    void DrawFinalShape(const TArray<FVector>& Shape);
    FVector GetLineIntersection(const FVector& Line1Start, const FVector& Line1End, 
                               const FVector& Line2Start, const FVector& Line2End) const;
    
    FVector SnapToGrid(const FVector& WorldPosition) const;
    FVector SnapToLocalGrid(const FVector& LocalPosition) const;
    FVector WorldToLocal(const FVector& WorldPosition) const;
    FVector LocalToWorld(const FVector& LocalPosition) const;
    bool IsValidGridPosition(const FVector& GridPosition) const;
    bool WouldCreateCrossingEdge(const FVector& NewPoint) const;
    bool DoLinesIntersect(const FVector& Line1Start, const FVector& Line1End, 
                         const FVector& Line2Start, const FVector& Line2End) const;
    
    TArray<FVector> GridPoints;
    TMap<FVector, bool> GridPointAvailability;
    
    UPROPERTY(BlueprintReadOnly)
    FBox GridBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    UStaticMesh* SphereMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    UStaticMesh* CylinderMesh;
};