#pragma once
#ifdef CTR_PLATFORM

#include "lwjgl/Keyboard.h"

// Synthetic key codes for raw 3DS pad buttons.
//
// GameSettings' KeyBinding::keyCode is just an int shared with the real
// lwjgl keyboard codes (0..KEY_MAX). Reusing that same field to name a pad
// button -- instead of adding a parallel "which button" field -- means the
// existing Controls screen, its rebind flow, options.txt persistence and
// GameSettings::keyName() all work for pad buttons with zero changes: they
// just see an int they don't recognize as a "real" key and Keyboard_3ds's
// getKeyName()/dsPadKeyName() treat it specially. This is the PS2's
// Ps2PadKeyCodes contract verbatim; only the button list differs.
//
// Only buttons that carry a rebindable game action are exposed. START stays
// reserved as the pause/menu key (DsInput turns its edges into KEY_ESCAPE,
// so a rebind cannot take it away from the player), and ZL/ZR are New-3DS
// only hardware -- deliberately absent while Old 3DS/2DS is the floor; they
// join this table when phase 2 maps them.
enum DsPadKeyCode : int
{
	DS_KEY_A = lwjgl::Keyboard::KEY_MAX,
	DS_KEY_B,
	DS_KEY_X,
	DS_KEY_Y,
	DS_KEY_L,
	DS_KEY_R,
	DS_KEY_SELECT,
	DS_KEY_DPAD_UP,
	DS_KEY_DPAD_DOWN,
	DS_KEY_DPAD_LEFT,
	DS_KEY_DPAD_RIGHT,
	DS_KEY_SENTINEL_END
};

static_assert(DS_KEY_SENTINEL_END < 256, "DsPadKeyCode must fit the 256-slot key-state arrays");

// Display name for the Controls screen, or nullptr if `key` isn't one of these.
const char *dsPadKeyName(int key);

#endif // CTR_PLATFORM
