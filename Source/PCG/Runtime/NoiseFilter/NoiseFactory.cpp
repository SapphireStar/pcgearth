#include "NoiseFactory.h"

#include "RigidNoiseFilter.h"
#include "SimpleNoiseFilter.h"

TSharedPtr<INoiseFilterInterface> NoiseFactory::CreateNoiseFilter(FNoiseLayer NoiseLayer)
{
	switch (NoiseLayer.NoiseType)
	{
	case ENoiseType::SGT_SimpleNoise:
		return MakeShared<SimpleNoiseFilter>(NoiseLayer);
		break;
	case ENoiseType::SGT_RigidNoise:
		return MakeShared<RigidNoiseFilter>(NoiseLayer);
		break;
	default: ;
	}
	return MakeShared<SimpleNoiseFilter>(NoiseLayer);
}
