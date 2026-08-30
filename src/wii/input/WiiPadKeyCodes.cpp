#ifdef WII_PLATFORM

#include "wii/input/WiiPadKeyCodes.h"

#include <gccore.h>
#include <wiiuse/wpad.h>

const char *wiiPadKeyName(int key)
{
	switch (key)
	{
		case WII_KEY_GC_A: return "GC A";
		case WII_KEY_GC_B: return "GC B";
		case WII_KEY_GC_X: return "GC X";
		case WII_KEY_GC_Y: return "GC Y";
		case WII_KEY_GC_Z: return "GC Z";
		case WII_KEY_GC_L: return "GC L";
		case WII_KEY_GC_R: return "GC R";
		case WII_KEY_WM_A: return "Wiimote A";
		case WII_KEY_WM_B: return "Wiimote B";
		case WII_KEY_WM_1: return "Wiimote 1";
		case WII_KEY_WM_2: return "Wiimote 2";
		case WII_KEY_WM_MINUS: return "Wiimote -";
		case WII_KEY_NUNCHUK_C: return "Nunchuk C";
		case WII_KEY_NUNCHUK_Z: return "Nunchuk Z";
		case WII_KEY_CC_A: return "Classic A";
		case WII_KEY_CC_B: return "Classic B";
		case WII_KEY_CC_X: return "Classic X";
		case WII_KEY_CC_Y: return "Classic Y";
		case WII_KEY_CC_MINUS: return "Classic -";
		case WII_KEY_CC_ZL: return "Classic ZL";
		case WII_KEY_CC_ZR: return "Classic ZR";
		default: return nullptr;
	}
}

const WiiPadFamily *wiiPadKeyFamily(int key)
{
	static const WiiPadFamily kGameCube = WiiPadFamily::GameCube;
	static const WiiPadFamily kWiimote = WiiPadFamily::Wiimote;
	static const WiiPadFamily kClassic = WiiPadFamily::Classic;
	switch (key)
	{
		case WII_KEY_GC_A: case WII_KEY_GC_B: case WII_KEY_GC_X: case WII_KEY_GC_Y:
		case WII_KEY_GC_Z: case WII_KEY_GC_L: case WII_KEY_GC_R:
			return &kGameCube;
		case WII_KEY_WM_A: case WII_KEY_WM_B: case WII_KEY_WM_1: case WII_KEY_WM_2: case WII_KEY_WM_MINUS:
		case WII_KEY_NUNCHUK_C: case WII_KEY_NUNCHUK_Z:
			return &kWiimote;
		case WII_KEY_CC_A: case WII_KEY_CC_B: case WII_KEY_CC_X: case WII_KEY_CC_Y: case WII_KEY_CC_MINUS:
		case WII_KEY_CC_ZL: case WII_KEY_CC_ZR:
			return &kClassic;
		default:
			return nullptr;
	}
}

const char *wiiPadButtonName(WiiPadFamily family, unsigned mask)
{
	if (mask == 0)
		return "(none)";
	switch (family)
	{
		case WiiPadFamily::GameCube:
			if (mask == PAD_BUTTON_A) return "A";
			if (mask == PAD_BUTTON_B) return "B";
			if (mask == PAD_BUTTON_X) return "X";
			if (mask == PAD_BUTTON_Y) return "Y";
			if (mask == PAD_TRIGGER_Z) return "Z";
			if (mask == PAD_TRIGGER_L) return "L";
			if (mask == PAD_TRIGGER_R) return "R";
			break;
		case WiiPadFamily::Wiimote:
			if (mask == WPAD_BUTTON_A) return "A";
			if (mask == WPAD_BUTTON_B) return "B";
			if (mask == WPAD_BUTTON_1) return "1";
			if (mask == WPAD_BUTTON_2) return "2";
			if (mask == WPAD_BUTTON_MINUS) return "-";
			if (mask == WPAD_NUNCHUK_BUTTON_C) return "Nun-C";
			if (mask == WPAD_NUNCHUK_BUTTON_Z) return "Nun-Z";
			break;
		case WiiPadFamily::Classic:
			if (mask == WPAD_CLASSIC_BUTTON_A) return "A";
			if (mask == WPAD_CLASSIC_BUTTON_B) return "B";
			if (mask == WPAD_CLASSIC_BUTTON_X) return "X";
			if (mask == WPAD_CLASSIC_BUTTON_Y) return "Y";
			if (mask == WPAD_CLASSIC_BUTTON_MINUS) return "-";
			if (mask == WPAD_CLASSIC_BUTTON_ZL) return "ZL";
			if (mask == WPAD_CLASSIC_BUTTON_ZR) return "ZR";
			break;
	}
	return nullptr;
}

unsigned wiiPadKeyButtonMask(int key)
{
	switch (key)
	{
		case WII_KEY_GC_A: return PAD_BUTTON_A;
		case WII_KEY_GC_B: return PAD_BUTTON_B;
		case WII_KEY_GC_X: return PAD_BUTTON_X;
		case WII_KEY_GC_Y: return PAD_BUTTON_Y;
		case WII_KEY_GC_Z: return PAD_TRIGGER_Z;
		case WII_KEY_GC_L: return PAD_TRIGGER_L;
		case WII_KEY_GC_R: return PAD_TRIGGER_R;
		case WII_KEY_WM_A: return WPAD_BUTTON_A;
		case WII_KEY_WM_B: return WPAD_BUTTON_B;
		case WII_KEY_WM_1: return WPAD_BUTTON_1;
		case WII_KEY_WM_2: return WPAD_BUTTON_2;
		case WII_KEY_WM_MINUS: return WPAD_BUTTON_MINUS;
		case WII_KEY_NUNCHUK_C: return WPAD_NUNCHUK_BUTTON_C;
		case WII_KEY_NUNCHUK_Z: return WPAD_NUNCHUK_BUTTON_Z;
		case WII_KEY_CC_A: return WPAD_CLASSIC_BUTTON_A;
		case WII_KEY_CC_B: return WPAD_CLASSIC_BUTTON_B;
		case WII_KEY_CC_X: return WPAD_CLASSIC_BUTTON_X;
		case WII_KEY_CC_Y: return WPAD_CLASSIC_BUTTON_Y;
		case WII_KEY_CC_MINUS: return WPAD_CLASSIC_BUTTON_MINUS;
		case WII_KEY_CC_ZL: return WPAD_CLASSIC_BUTTON_ZL;
		case WII_KEY_CC_ZR: return WPAD_CLASSIC_BUTTON_ZR;
		default: return 0;
	}
}

#endif // WII_PLATFORM
