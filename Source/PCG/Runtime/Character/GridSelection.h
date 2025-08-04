// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TextRenderActor.h"
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
    float SelectedPointRadius = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float SelectedGridsThickness = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float PreviewLineRadius = 0.1f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Settings")
    FVector GridCenter = FVector::ZeroVector;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Settings")
    FRotator GridRotation = FRotator::ZeroRotator;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* GridPlaneMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* SelectedGridMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* SelectedPointMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* PreviewLineMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterial* SelectedGridMaterial;

    UPROPERTY()
    UMaterialInstanceDynamic* SelectedGridMaterialInstance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AvailableColor = FLinearColor::Green;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor UnavailableColor = FLinearColor::Red;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterial* SelectedPointMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterial* PreviewLineMaterial;

    UPROPERTY(BlueprintReadOnly)
    bool bIsInGridSelectionMode = false;
    
    UPROPERTY(BlueprintReadOnly)
    FVector InitialSelectedPoint;
    
    UPROPERTY(BlueprintReadOnly)
    TArray<FVector> SelectedGridPoints;

    UPROPERTY(BlueprintReadOnly)
    UStaticMeshComponent* SelectedGrid;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UStaticMeshComponent*> SelectedPoints;
    
    UPROPERTY(BlueprintReadOnly)
    TArray<UStaticMeshComponent*> PreviewLines;
    
    UPROPERTY(BlueprintReadOnly)
    FBox GridBounds;

    TArray<FVector> GridPoints;
    
    TMap<FVector, bool> GridPointAvailability;

    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UTextRenderComponent> TextRenderer;
public:
    UFUNCTION(BlueprintCallable)
    void StartGridSelection(const FVector& StartPoint,const FVector& Normal, const FRotator& Rotation = FRotator::ZeroRotator);
    
    UFUNCTION(BlueprintCallable)
    FBox EndGridSelection();

    UFUNCTION(BlueprintCallable)
    FBox PeekGridSelection();

    UFUNCTION(BlueprintCallable)
    FIntVector PeekGridSize();

    UFUNCTION(BlueprintCallable)
    void ShutDownGridSelection();

    UFUNCTION(BlueprintCallable)
    FRotator GetGridRotation() const{return GridRotation;}

    UFUNCTION(BlueprintCallable)
    float GetGridSize() const {return  GridSize;}

    UFUNCTION(BlueprintCallable)
    bool PreviewSelectGrid(const FVector& WorldPosition);

    UFUNCTION(BlueprintCallable)
    void ClearSelection();
    
    UFUNCTION(BlueprintCallable)
    TArray<FVector> GetFinalGrid() const;

    void SetGridUnavailable();
    void SetGridAvailable();

protected:
    void GenerateGrid();

    void ShowPreviewMeshes();
    void HidePreviewMeshes();
    void DrawSelectedGrid();
    void DrawSelectedPoints();
    void DrawPreviewLines();
    
    FVector SnapToGrid(const FVector& WorldPosition) const;
    FVector SnapToLocalGrid(const FVector& LocalPosition) const;
    FVector WorldToLocal(const FVector& WorldPosition) const;
    FVector LocalToWorld(const FVector& LocalPosition) const;
    FRotator CalculatePlaneNormalRotation(FVector Center, FVector Normal,  FRotator Rotation);
};