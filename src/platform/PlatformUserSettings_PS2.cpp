#include "platform/PlatformUserSettings.h"

#include "ps2/input/Ps2AnalogFilter.h"

namespace PlatformUserSettings
{
void setControllerDeadzone(float value)
{
	Ps2AnalogFilter::setDeadzone(value);
}

void setAlternativeControls(bool) {}
}
