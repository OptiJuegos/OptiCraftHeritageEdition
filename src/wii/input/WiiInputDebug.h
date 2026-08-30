#pragma once

#ifdef WII_PLATFORM

#include <cstdint>

struct WiiInputDebugState
{
	bool gcConnected = false;
	std::uint16_t gcButtons = 0;
	int gcStickX = 0;
	int gcStickY = 0;

	bool wmConnected = false;
	std::uint32_t wmButtons = 0;
	int expType = 0;
	int expProbed = 0;

	float jsAng = 0.0f;
	float jsMag = 0.0f;
	int jsRawX = 0;
	int jsRawY = 0;
	int jsCenterX = 0;
	int jsCenterY = 0;

	bool irValid = false;
	int irX = 0;
	int irY = 0;
	int irDots = 0;
	int irMag = 0;
};

WiiInputDebugState& wiiInputDebugState();
void wiiInputDebugReset();
const char* wiiPadDebugLine();

#endif
