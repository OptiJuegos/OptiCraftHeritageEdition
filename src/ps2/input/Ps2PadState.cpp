#include "ps2/input/Ps2PadState.h"

#ifdef PS2_PLATFORM

static Ps2PadSnapshot s_ps2PadSnapshot[PS2_MAX_PADS] = {
	{ false, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0 },
	{ false, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0 }
};

static unsigned short s_latchedPressed[PS2_MAX_PADS] = {};

const Ps2PadSnapshot& ps2PadGetSnapshot(int port)
{
	if (port < 0 || port >= PS2_MAX_PADS)
		port = 0;
	return s_ps2PadSnapshot[port];
}

unsigned short ps2PadConsumePressed(int port)
{
	if (port < 0 || port >= PS2_MAX_PADS)
		port = 0;
	const unsigned short pressed = s_latchedPressed[port];
	s_latchedPressed[port] = 0;
	return pressed;
}

void ps2PadLatchPressed(int port, unsigned short pressed)
{
	if (port < 0 || port >= PS2_MAX_PADS)
		return;
	s_latchedPressed[port] |= pressed;
}

void ps2PadClearLatchedPressed(int port)
{
	if (port < 0)
	{
		for (int i = 0; i < PS2_MAX_PADS; ++i)
			s_latchedPressed[i] = 0;
	}
	else if (port < PS2_MAX_PADS)
	{
		s_latchedPressed[port] = 0;
	}
}

void ps2PadUpdateSnapshot(int port, bool connected, float leftX, float leftY, float rightX, float rightY,
	unsigned short held, unsigned short pressed, unsigned short released)
{
	if (port < 0 || port >= PS2_MAX_PADS)
		return;
	Ps2PadSnapshot& s = s_ps2PadSnapshot[port];
	s_latchedPressed[port] |= pressed;
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
	if (port < 0 || port >= PS2_MAX_PADS)
		return;
	s_latchedPressed[port] = 0;
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
