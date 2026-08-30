#ifdef WII_PLATFORM

#include "wii/input/WiiInputDebug.h"

#include <cstdio>

namespace
{
WiiInputDebugState g_state;
char g_line[224];
}

WiiInputDebugState& wiiInputDebugState()
{
	return g_state;
}

void wiiInputDebugReset()
{
	g_state = WiiInputDebugState();
}

const char* wiiPadDebugLine()
{
	static const char* const kExpansionNames[] = {
		"none", "nunchuk", "classic", "gh3", "board"
	};
	const char* expansion = (g_state.expType >= 0 && g_state.expType <= 4)
		? kExpansionNames[g_state.expType]
		: "?";

	std::snprintf(g_line, sizeof(g_line),
		"GC%c b%04X s%d,%d | WM%c b%08lX %s(p%d) | ir%c %d,%d d%d m%d | js a%d m%d r%d,%d c%d,%d",
		g_state.gcConnected ? '+' : '-',
		(unsigned)g_state.gcButtons, g_state.gcStickX, g_state.gcStickY,
		g_state.wmConnected ? '+' : '-',
		(unsigned long)g_state.wmButtons, expansion, g_state.expProbed,
		g_state.irValid ? '+' : '-', g_state.irX, g_state.irY, g_state.irDots, g_state.irMag,
		(int)g_state.jsAng, (int)(g_state.jsMag * 100.0f),
		g_state.jsRawX, g_state.jsRawY, g_state.jsCenterX, g_state.jsCenterY);
	return g_line;
}

#endif
