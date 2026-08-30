#pragma once

#ifdef PS2_PLATFORM

enum Ps2PadButton {
	PS2_PAD_SELECT   = 0x0001,
	PS2_PAD_L3       = 0x0002,
	PS2_PAD_R3       = 0x0004,
	PS2_PAD_START    = 0x0008,
	PS2_PAD_UP       = 0x0010,
	PS2_PAD_RIGHT    = 0x0020,
	PS2_PAD_DOWN     = 0x0040,
	PS2_PAD_LEFT     = 0x0080,
	PS2_PAD_L2       = 0x0100,
	PS2_PAD_R2       = 0x0200,
	PS2_PAD_L1       = 0x0400,
	PS2_PAD_R1       = 0x0800,
	PS2_PAD_TRIANGLE = 0x1000,
	PS2_PAD_CIRCLE   = 0x2000,
	PS2_PAD_CROSS    = 0x4000,
	PS2_PAD_SQUARE   = 0x8000
};

struct Ps2PadSnapshot {
	bool connected;
	float leftX;
	float leftY;
	float rightX;
	float rightY;
	unsigned short held;
	unsigned short pressed;
	unsigned short released;
};

// Two controller ports: port 0 = player 1, port 1 = player 2 (splitscreen).
#define PS2_MAX_PADS 2

const Ps2PadSnapshot& ps2PadGetSnapshot(int port = 0);
void ps2PadUpdateSnapshot(int port, bool connected, float leftX, float leftY, float rightX, float rightY,
	unsigned short held, unsigned short pressed, unsigned short released);
void ps2PadDisconnect(int port = 0);

// Which pad drives the simulated menu cursor (splitscreen: pad 2 while a
// player-2-owned screen is open). Default 0.
int  ps2GetMenuPad();
void ps2SetMenuPad(int port);

// The pad that opened the current menu (set when Escape opens a screen,
// cleared when the screen closes). Takes priority over ps2GetMenuPad().
// -1 = no owner set, fall back to ps2GetMenuPad().
int  ps2GetMenuOwnerPad();
void ps2SetMenuOwnerPad(int port);

#endif
