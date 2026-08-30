#pragma once

#ifdef WII_PLATFORM

#include <cstdint>

// Controller actions reserved for the shared GUI helpers: the on-screen keyboard
// owns the whole set while a text field is focused, and the container slot
// navigation reads the four direction bits while an inventory-style screen is
// open. The snapshot is assembled from GameCube, Wiimote and Classic Controller,
// so neither has to care which controller is connected.
enum WiiTextKey : std::uint32_t
{
	WII_TEXT_LEFT   = 1u << 0,
	WII_TEXT_RIGHT  = 1u << 1,
	WII_TEXT_UP     = 1u << 2,
	WII_TEXT_DOWN   = 1u << 3,
	WII_TEXT_TYPE   = 1u << 4,
	WII_TEXT_BACK   = 1u << 5,
	WII_TEXT_SPACE  = 1u << 6,
	WII_TEXT_SHIFT  = 1u << 7,
	WII_TEXT_ENTER  = 1u << 8,
	WII_TEXT_CLOSE  = 1u << 9,
};

struct WiiTextInputSnapshot
{
	std::uint32_t held = 0;
	std::uint32_t pressed = 0;
	bool irValid = false;
	int irX = 0;
	int irY = 0;
	int irWidth = 0;
	int irHeight = 0;
};

struct WiiStickSnapshot
{
	bool connected = false;
	float x = 0.0f;
	float y = 0.0f;
};

WiiTextInputSnapshot wiiTextInputSnapshot();
WiiStickSnapshot wiiStickSnapshot();

// Place the simulated GUI cursor at an absolute framebuffer position. Used by
// the container slot navigation to step between slots; the IR pointer and the
// sticks overwrite it again as soon as the player touches one.
void wiiSetCursorPosition(int x, int y);

void wiiSetAlternativeControls(bool enabled);
void wiiSetStickDeadzone(float deadzone);

#endif
