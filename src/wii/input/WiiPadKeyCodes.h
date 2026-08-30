#pragma once
#ifdef WII_PLATFORM

#include "lwjgl/Keyboard.h"

// Synthetic key codes for the Wii pad buttons a Controls-menu binding can
// learn, one set per controller family (GameCube pad / Wiimote's own buttons
// / Classic Controller). Used only as the vehicle for capturing a rebind --
// see Ps2PadKeyCodes.h for the reasoning, which applies the same way here.
// Ongoing gameplay reads WiiButtonBindings' raw masks directly, never these.
//
// Attack/Use/ThirdPerson are rebindable too, so each family's shoulder
// trigger (or the Nunchuk's C/Z on the Wiimote side) is offered alongside
// its face buttons -- hotbar scroll stays wherever it already lived
// (D-pad/ZL/ZR) since nothing here reassigns it.
enum WiiPadKeyCode : int
{
	WII_KEY_GC_A = lwjgl::Keyboard::KEY_MAX,
	WII_KEY_GC_B,
	WII_KEY_GC_X,
	WII_KEY_GC_Y,
	WII_KEY_GC_Z,
	WII_KEY_GC_L,
	WII_KEY_GC_R,
	WII_KEY_WM_A,
	WII_KEY_WM_B,
	WII_KEY_WM_1,
	WII_KEY_WM_2,
	WII_KEY_WM_MINUS,
	WII_KEY_NUNCHUK_C,
	WII_KEY_NUNCHUK_Z,
	WII_KEY_CC_A,
	WII_KEY_CC_B,
	WII_KEY_CC_X,
	WII_KEY_CC_Y,
	WII_KEY_CC_MINUS,
	WII_KEY_CC_ZL,
	WII_KEY_CC_ZR,
	WII_KEY_SENTINEL_END
};

static_assert(WII_KEY_SENTINEL_END < 256, "WiiPadKeyCode must fit the 256-slot key-state arrays");

enum class WiiPadFamily { GameCube, Wiimote, Classic };

// Display name for the Controls screen, or nullptr if `key` isn't one of these.
const char *wiiPadKeyName(int key);

// Which family a captured code belongs to, or nullptr if `key` isn't one of these.
const WiiPadFamily *wiiPadKeyFamily(int key);

// The live PAD_*/WPAD_* bit for one of these codes, or 0 if `key` isn't one.
unsigned wiiPadKeyButtonMask(int key);

// Reverse of wiiPadKeyButtonMask: the short display name for a raw
// PAD_*/WPAD_* bit within one family, or nullptr if it isn't a bit this file
// knows about (e.g. a mask of 0, meaning "no button assigned").
const char *wiiPadButtonName(WiiPadFamily family, unsigned mask);

#endif // WII_PLATFORM
