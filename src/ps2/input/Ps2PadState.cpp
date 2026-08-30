#include "ps2/input/Ps2PadState.h"

#ifdef PS2_PLATFORM

static Ps2PadSnapshot s_ps2PadSnapshot[PS2_MAX_PADS] = {
	{ false, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0 },
	{ false, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0 }
};

const Ps2PadSnapshot& ps2PadGetSnapshot(int port)
{
	if (port < 0 || port >= PS2_MAX_PADS)
		port = 0;
	return s_ps2PadSnapshot[port];
}

void ps2PadUpdateSnapshot(int port, bool connected, float leftX, float leftY, float rightX, float rightY,
	unsigned short held, unsigned short pressed, unsigned short released)
{
	if (port < 0 || port >= PS2_MAX_PADS)
		return;
	Ps2PadSnapshot& s = s_ps2PadSnapshot[port];
	s.connected = connected;
	s.leftX = leftX;
	s.leftY = leftY;
	s.rightX = rightX;
	s.rightY = rightY;
	s.held = held;
	s.pressed = pressed;
	s.released = released;
}

void ps2PadDisconnect(int port)
{
	ps2PadUpdateSnapshot(port, false, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0);
}

static int s_menuPad = 0;
static int s_menuOwnerPad = -1;

int ps2GetMenuPad()
{
	return s_menuPad;
}

void ps2SetMenuPad(int port)
{
	s_menuPad = (port >= 0 && port < PS2_MAX_PADS) ? port : 0;
}

int ps2GetMenuOwnerPad()
{
	return s_menuOwnerPad;
}

void ps2SetMenuOwnerPad(int port)
{
	s_menuOwnerPad = (port >= 0 && port < PS2_MAX_PADS) ? port : -1;
}

#endif
