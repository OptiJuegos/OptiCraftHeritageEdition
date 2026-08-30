#include "platform/PlatformUserSettings.h"

#include "wii/input/WiiPadState.h"

namespace PlatformUserSettings
{
void setControllerDeadzone(float value)
{
	wiiSetStickDeadzone(value);
}

void setAlternativeControls(bool enabled)
{
	wiiSetAlternativeControls(enabled);
}
}
