#ifdef WII_PLATFORM

#include "wii/input/WiiButtonBindings.h"

#include <gccore.h>
#include <wiiuse/wpad.h>

namespace WiiButtonBindings
{
namespace
{
	// GameCube and Classic reproduce what WiiGameCubePad.cpp / WiiRemote.cpp
	// used to hardcode before per-family rebinding existed. The Wiimote
	// layout instead ships the A/B/C/Z rotation requested for comfortable
	// play with alternative movement: Sneak on A (previously unbound without
	// a Nunchuk), Jump on B (previously Attack), Attack on Nunchuk C
	// (previously Sneak), Use unchanged on Nunchuk Z.
	Snapshot s_snapshot = {
		{ PAD_BUTTON_A, PAD_BUTTON_B, PAD_BUTTON_Y, PAD_BUTTON_X, PAD_TRIGGER_R, PAD_TRIGGER_L, PAD_BUTTON_DOWN },
		{ WPAD_BUTTON_B, WPAD_BUTTON_A, WPAD_BUTTON_2, WPAD_BUTTON_MINUS, WPAD_NUNCHUK_BUTTON_C, WPAD_NUNCHUK_BUTTON_Z, 0 },
		{ WPAD_CLASSIC_BUTTON_A, WPAD_CLASSIC_BUTTON_B, WPAD_CLASSIC_BUTTON_Y, WPAD_CLASSIC_BUTTON_X,
		  WPAD_CLASSIC_BUTTON_FULL_R, WPAD_CLASSIC_BUTTON_FULL_L, 0 },
	};
}

void set(const Snapshot &snapshot)
{
	s_snapshot = snapshot;
}

const Snapshot &get()
{
	return s_snapshot;
}

}

#endif // WII_PLATFORM
