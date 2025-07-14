#pragma once

class MinMax
{
public:
	float Min = FLT_MAX;
	float Max = FLT_MIN;
	void AddValue(float v);
};
