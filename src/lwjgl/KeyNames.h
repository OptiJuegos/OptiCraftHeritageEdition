#pragma once

// Shared key-name lookup for platforms with no OS-level "give me a label for
// this key" API (PS2, Wii — PC uses SDL_GetKeyName instead). Covers the keys
// reachable from GameSettings' default and common rebinding choices; anything
// else is left to the caller to fall back on.

#include "lwjgl/Keyboard.h"

namespace lwjgl
{
namespace Keyboard
{

inline const char *lwjglKeyDisplayName(int_t key)
{
	switch (key)
	{
		case KEY_ESCAPE: return "ESC";
		case KEY_1: return "1";
		case KEY_2: return "2";
		case KEY_3: return "3";
		case KEY_4: return "4";
		case KEY_5: return "5";
		case KEY_6: return "6";
		case KEY_7: return "7";
		case KEY_8: return "8";
		case KEY_9: return "9";
		case KEY_0: return "0";
		case KEY_MINUS: return "-";
		case KEY_EQUALS: return "=";
		case KEY_BACK: return "BACKSPACE";
		case KEY_TAB: return "TAB";
		case KEY_Q: return "Q";
		case KEY_W: return "W";
		case KEY_E: return "E";
		case KEY_R: return "R";
		case KEY_T: return "T";
		case KEY_Y: return "Y";
		case KEY_U: return "U";
		case KEY_I: return "I";
		case KEY_O: return "O";
		case KEY_P: return "P";
		case KEY_LBRACKET: return "[";
		case KEY_RBRACKET: return "]";
		case KEY_RETURN: return "ENTER";
		case KEY_LCONTROL: return "L-CTRL";
		case KEY_A: return "A";
		case KEY_S: return "S";
		case KEY_D: return "D";
		case KEY_F: return "F";
		case KEY_G: return "G";
		case KEY_H: return "H";
		case KEY_J: return "J";
		case KEY_K: return "K";
		case KEY_L: return "L";
		case KEY_SEMICOLON: return ";";
		case KEY_APOSTROPHE: return "'";
		case KEY_GRAVE: return "`";
		case KEY_LSHIFT: return "L-SHIFT";
		case KEY_BACKSLASH: return "\\";
		case KEY_Z: return "Z";
		case KEY_X: return "X";
		case KEY_C: return "C";
		case KEY_V: return "V";
		case KEY_B: return "B";
		case KEY_N: return "N";
		case KEY_M: return "M";
		case KEY_COMMA: return ",";
		case KEY_PERIOD: return ".";
		case KEY_SLASH: return "/";
		case KEY_RSHIFT: return "R-SHIFT";
		case KEY_MULTIPLY: return "NUM *";
		case KEY_LMENU: return "L-ALT";
		case KEY_SPACE: return "SPACE";
		case KEY_CAPITAL: return "CAPS LOCK";
		case KEY_F1: return "F1";
		case KEY_F2: return "F2";
		case KEY_F3: return "F3";
		case KEY_F4: return "F4";
		case KEY_F5: return "F5";
		case KEY_F6: return "F6";
		case KEY_F7: return "F7";
		case KEY_F8: return "F8";
		case KEY_F9: return "F9";
		case KEY_F10: return "F10";
		case KEY_NUMLOCK: return "NUM LOCK";
		case KEY_SCROLL: return "SCROLL LOCK";
		case KEY_NUMPAD7: return "NUM 7";
		case KEY_NUMPAD8: return "NUM 8";
		case KEY_NUMPAD9: return "NUM 9";
		case KEY_SUBTRACT: return "NUM -";
		case KEY_NUMPAD4: return "NUM 4";
		case KEY_NUMPAD5: return "NUM 5";
		case KEY_NUMPAD6: return "NUM 6";
		case KEY_ADD: return "NUM +";
		case KEY_NUMPAD1: return "NUM 1";
		case KEY_NUMPAD2: return "NUM 2";
		case KEY_NUMPAD3: return "NUM 3";
		case KEY_NUMPAD0: return "NUM 0";
		case KEY_DECIMAL: return "NUM .";
		case KEY_F11: return "F11";
		case KEY_F12: return "F12";
		case KEY_NUMPADENTER: return "NUM ENTER";
		case KEY_RCONTROL: return "R-CTRL";
		case KEY_DIVIDE: return "NUM /";
		case KEY_RMENU: return "R-ALT";
		case KEY_PAUSE: return "PAUSE";
		case KEY_HOME: return "HOME";
		case KEY_UP: return "UP";
		case KEY_PRIOR: return "PAGE UP";
		case KEY_LEFT: return "LEFT";
		case KEY_RIGHT: return "RIGHT";
		case KEY_END: return "END";
		case KEY_DOWN: return "DOWN";
		case KEY_NEXT: return "PAGE DOWN";
		case KEY_INSERT: return "INSERT";
		case KEY_DELETE: return "DELETE";
		default: return nullptr;
	}
}

}
}
