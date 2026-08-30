#pragma once

#ifdef PS2_PLATFORM

namespace Ps2AnalogFilter
{
void setDeadzone(float deadzone);
float getDeadzone();
float apply(float value);
}

#endif
