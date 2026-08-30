#ifdef WII_PLATFORM
#include "wii/system/WiiSystemEvents.h"

#include <gccore.h>
#include <wiiuse/wpad.h>

namespace
{
volatile bool g_exitRequested = false;
void onResetPressed(u32, void*) { g_exitRequested = true; }
void onPowerPressed() { g_exitRequested = true; }
void onWiimotePower(s32) { g_exitRequested = true; }
}

namespace WiiSystemEvents
{
void install()
{
	SYS_SetResetCallback(onResetPressed);
	SYS_SetPowerCallback(onPowerPressed);
	WPAD_SetPowerButtonCallback(onWiimotePower);
}

bool exitRequested() { return g_exitRequested; }
void clearExitRequest() { g_exitRequested = false; }
}
#endif
