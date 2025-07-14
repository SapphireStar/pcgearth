#include "MinMax.h"

void MinMax::AddValue(float v)
{
	if (v>Max)
	{
		Max = v;
	}
	if (v<Min)
	{
		Min = v;
	}
}
