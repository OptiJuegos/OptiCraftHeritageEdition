#include "platform/PlatformUserSettings.h"

namespace PlatformUserSettings
{
namespace
{
// File-local stores rather than empty setters: the options screen's writes
// must survive the call so the 3DS input and renderer backends -- which land
// in a later phase -- read the player's choices instead of defaults. No Wii or
// PS2 symbol is involved; both of those forward straight into their backend.
//
// Defaults are the desktop's neutral settings: no deadzone trim, standard
// controls, deflicker off (a Wii display-filter feature the 3DS has no path
// for, stored only to honour the header's contract).
float g_controllerDeadzone = 0.0f;
bool g_alternativeControls = false;
bool g_displayDeflicker = false;
}  // namespace

void setControllerDeadzone(float value)
{
	g_controllerDeadzone = value;
}

void setAlternativeControls(bool enabled)
{
	g_alternativeControls = enabled;
}

void setDisplayDeflicker(bool enabled)
{
	g_displayDeflicker = enabled;
}
}
