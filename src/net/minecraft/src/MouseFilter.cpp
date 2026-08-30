#include "MouseFilter.h"

MouseFilter::MouseFilter() :
	field_22388_a(0.0f),
	field_22387_b(0.0f),
	field_22389_c(0.0f)
{
}

float MouseFilter::smooth(float f, float f1)
{
	field_22388_a += f;
	f = (field_22388_a - field_22387_b) * f1;
	field_22389_c = field_22389_c + (f - field_22389_c) * 0.5f;
	if (f > 0.0f && f > field_22389_c || f < 0.0f && f < field_22389_c)
	{
		f = field_22389_c;
	}
	field_22387_b += f;
	return f;
}
