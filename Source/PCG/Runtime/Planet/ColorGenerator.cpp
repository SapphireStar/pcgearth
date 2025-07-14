#include "ColorGenerator.h"

UColorGenerator::UColorGenerator(FColorSettings Settings)
{
	this->ColorSettings = Settings; 
}

void UColorGenerator::UpdateElevation(TSharedPtr<MinMax> elevationMinMax, UMaterialInstanceDynamic* MaterialInstanceDynamic)
{
	MaterialInstanceDynamic->SetVectorParameterValue(FName(TEXT("ElevationMinMax")), FLinearColor(elevationMinMax->Min, elevationMinMax->Max, 0, 0));
}
