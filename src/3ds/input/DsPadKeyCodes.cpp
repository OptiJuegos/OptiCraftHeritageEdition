#ifdef CTR_PLATFORM

#include "3ds/input/DsPadKeyCodes.h"

const char *dsPadKeyName(int key)
{
	switch (key)
	{
		case DS_KEY_A: return "A";
		case DS_KEY_B: return "B";
		case DS_KEY_X: return "X";
		case DS_KEY_Y: return "Y";
		case DS_KEY_L: return "L";
		case DS_KEY_R: return "R";
		case DS_KEY_SELECT: return "SELECT";
		case DS_KEY_DPAD_UP: return "D-Pad Up";
		case DS_KEY_DPAD_DOWN: return "D-Pad Down";
		case DS_KEY_DPAD_LEFT: return "D-Pad Left";
		case DS_KEY_DPAD_RIGHT: return "D-Pad Right";
		default: return nullptr;
	}
}

#endif // CTR_PLATFORM
