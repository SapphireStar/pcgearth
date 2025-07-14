#pragma once

#include "CoreMinimal.h"
#include "WFCBlock.h"
#include "WFCData.h"
#include "GameFramework/Actor.h"
#include "GridSystem.generated.h"
UCLASS()
class PCG_API AGridSystem : public AActor
{
	GENERATED_BODY()

public:
	AGridSystem();
protected:
	virtual void BeginPlay() override;
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
	TObjectPtr<UStaticMesh> DefaultMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
	int32 GridSizeX = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
	int32 GridSizeY = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
	int32 GridSizeZ = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
	float CellSize = 100.0f;

	TArray<TArray<TArray<FVector>>> Grid;

	TArray<TArray<TArray<TObjectPtr<AWFCBlock>>>> GridCells;

public:
	UFUNCTION(BlueprintCallable)
	void InitializeGrid(int x, int y, int z);
	
	UFUNCTION(BlueprintCallable, Category = "Grid System")
	void SetGridCell(int32 X, int32 Y, int32 Z, AWFCBlock* NewCell);

	UFUNCTION(BlueprintCallable, Category = "Grid System")
	AWFCBlock* GetGridCell(int32 X, int32 Y, int32 Z);

private:

	UFUNCTION(BlueprintCallable)
	bool IsValidIndex(int32 X, int32 Y, int32 Z);

	UFUNCTION(BlueprintCallable)
	FVector CalculateWorldPosition(int32 X, int32 Y, int32 Z);

	UStaticMeshComponent* CreateMeshComponent(const FString& ComponentName);
	UFUNCTION(BlueprintCallable)
	AWFCBlock* CreateWFCBlock(int X, int Y, int Z, FVector Pos);

	void ClearGrid();
};
