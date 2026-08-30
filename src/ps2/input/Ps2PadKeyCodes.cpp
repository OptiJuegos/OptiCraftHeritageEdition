#ifdef PS2_PLATFORM

#include "ps2/input/Ps2PadKeyCodes.h"

const char *ps2PadKeyName(int key)
{
	switch (key)
	{
		case PS2_KEY_CROSS: return "Cross";
		case PS2_KEY_CIRCLE: return "Circle";
		case PS2_KEY_TRIANGLE: return "Triangle";
		case PS2_KEY_SQUARE: return "Square";
		case PS2_KEY_L1: return "L1";
		case PS2_KEY_R1: return "R1";
		case PS2_KEY_L2: return "L2";
		case PS2_KEY_R2: return "R2";
		case PS2_KEY_L3: return "L3";
		case PS2_KEY_R3: return "R3";
		case PS2_KEY_DPAD_UP: return "D-Pad Up";
		case PS2_KEY_DPAD_DOWN: return "D-Pad Down";
		case PS2_KEY_DPAD_LEFT: return "D-Pad Left";
		case PS2_KEY_DPAD_RIGHT: return "D-Pad Right";
		default: return nullptr;
	}
}

#endif // PS2_PLATFORM
