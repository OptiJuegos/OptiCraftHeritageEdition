#include "ps2/input/Ps2AnalogFilter.h"

#ifdef PS2_PLATFORM

namespace
{
float s_deadzone = 0.20f;

float clampDeadzone(float deadzone)
{
	if (deadzone < 0.05f)
		return 0.05f;
	if (deadzone > 0.35f)
		return 0.35f;
	return deadzone;
}
}

namespace Ps2AnalogFilter
{
void setDeadzone(float deadzone)
{
	s_deadzone = clampDeadzone(deadzone);
}

float getDeadzone()
{
	return s_deadzone;
}

float apply(float value)
{
	if (value > -s_deadzone && value < s_deadzone)
		return 0.0f;

	const float sign = value < 0.0f ? -1.0f : 1.0f;
	const float absolute = value * sign;
	float output = (absolute - s_deadzone) / (1.0f - s_deadzone);
	if (output < 0.0f)
		output = 0.0f;
	if (output > 1.0f)
		output = 1.0f;
	return output * sign;
}
}

#endif
