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

    // 网格参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float GridSize = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    int32 GridWidth = 20;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    int32 GridHeight = 20;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    FVector GridCenter = FVector::ZeroVector;
    
    // 可视化组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* GridPlaneMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // 材质和视觉效果
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* GridMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* SelectedPointMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* PreviewLineMaterial;

    // 选择状态
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
    // 公共接口
    UFUNCTION(BlueprintCallable, Category = "Grid Selection")
    void StartGridSelection(const FVector& StartPoint);
    
    UFUNCTION(BlueprintCallable, Category = "Grid Selection")
    void EndGridSelection();
    
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
    // 内部函数
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
    bool IsValidGridPosition(const FVector& GridPosition) const;
    bool WouldCreateCrossingEdge(const FVector& NewPoint) const;
    bool DoLinesIntersect(const FVector& Line1Start, const FVector& Line1End, 
                         const FVector& Line2Start, const FVector& Line2End) const;
    
    // 网格数据
    TArray<FVector> GridPoints;
    TMap<FVector, bool> GridPointAvailability;
    
    UPROPERTY(BlueprintReadOnly)
    FBox GridBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    UStaticMesh* SphereMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    UStaticMesh* CylinderMesh;
};
