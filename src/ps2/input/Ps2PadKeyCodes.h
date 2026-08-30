#pragma once
#ifdef PS2_PLATFORM

#include "lwjgl/Keyboard.h"

// Synthetic key codes for raw PS2 pad buttons.
//
// GameSettings' KeyBinding::keyCode is just an int shared with the real
// lwjgl keyboard codes (0..KEY_MAX). Reusing that same field to name a pad
// button -- instead of adding a parallel "which button" field -- means the
// existing Controls screen, its rebind flow, options.txt persistence, and
// GameSettings::keyName() all work for pad buttons with zero changes: they
// just see an int they don't recognize as a "real" key and Keyboard_ps2's
// getKeyName()/Ps2InputMapper treat it specially.
//
// Only the 14 buttons that have no fixed system role are exposed here.
// Start (pause) and Select (debug overlay) stay reserved so a rebind can't
// take them away from the player.
enum Ps2PadKeyCode : int
{
	PS2_KEY_CROSS = lwjgl::Keyboard::KEY_MAX,
	PS2_KEY_CIRCLE,
	PS2_KEY_TRIANGLE,
	PS2_KEY_SQUARE,
	PS2_KEY_L1,
	PS2_KEY_R1,
	PS2_KEY_L2,
	PS2_KEY_R2,
	PS2_KEY_L3,
	PS2_KEY_R3,
	PS2_KEY_DPAD_UP,
	PS2_KEY_DPAD_DOWN,
	PS2_KEY_DPAD_LEFT,
	PS2_KEY_DPAD_RIGHT,
	PS2_KEY_SENTINEL_END
};

static_assert(PS2_KEY_SENTINEL_END < 256, "Ps2PadKeyCode must fit the 256-slot key-state arrays");

// Display name for the Controls screen, or nullptr if `key` isn't one of these.
const char *ps2PadKeyName(int key);

#endif // PS2_PLATFORM
