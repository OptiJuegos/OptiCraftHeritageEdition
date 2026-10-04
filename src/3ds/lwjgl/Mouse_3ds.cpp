// Mouse_3ds.cpp — Nintendo 3DS implementation of lwjgl::Mouse.
//
// Structurally identical to the PS2/Wii versions (event queue + staged
// deltas), and deliberately so: this file only stores what it is given. What
// differs is the producer, src/3ds/input/DsInput.cpp:
//
//   PS2  the right analogue stick integrates a simulated cursor, so motion is
//        naturally relative and deltas fall out for free.
//   Wii  the Wiimote IR pointer is ABSOLUTE; the producer differences
//        successive positions to synthesise xrel/yrel.
//   3DS  the touch panel is ABSOLUTE in exactly the same way (bottom LCD,
//        already mapped to top-screen pixels by DsInput), so DsInput differs
//        touch positions the same way the Wii producer does and this file
//        receives the same shape of events: absolute position plus deltas.
//
// Coordinates arrive top-left origin, as on PS2/Wii. LWJGL exposes
// bottom-left, and the conversion happens in the getters below.
#ifdef CTR_PLATFORM

#include "lwjgl/Mouse.h"
#include "lwjgl/Display.h"

#include <queue>

namespace lwjgl
{
namespace Mouse
{

namespace detail
{

struct Event
{
	int button; // -1 = motion/wheel only
	int down;   //  0/1
	int x, y;
	int xrel, yrel;
	int wheel;
};

static Event             s_current = {};
static std::queue<Event> s_queue;
static int  s_stagingDX  = 0;
static int  s_stagingDY  = 0;
static int  s_stagingDW  = 0;
static int  s_cursorX    = 0;
static int  s_cursorY    = 0;
static bool s_grabbed    = false;
static bool s_btnDown[3] = {}; // left(0), right(1), middle(2)

void pushMotion(int x, int y, int xrel, int yrel)
{
	// The touch pointer is absolute: (x,y) is where the finger is now, while
	// xrel/yrel is the difference DsInput computed — both are stored, since
	// menus read the position and in-game look reads the deltas.
	s_stagingDX += xrel;
	s_stagingDY -= yrel;
	s_cursorX = x;
	s_cursorY = y;
	s_queue.push({-1, 0, x, y, xrel, yrel, 0});
}

void pushButton(int button, bool down, int x, int y)
{
	if (button >= 0 && button < 3)
		s_btnDown[button] = down;
	s_cursorX = x;
	s_cursorY = y;
	s_queue.push({button, down ? 1 : 0, x, y, 0, 0, 0});
}

void pushWheel(int delta, int x, int y)
{
	// The 3DS has no wheel (and phase 1 maps no button to one); kept so the
	// console detail interface stays uniform across platforms.
	s_stagingDW += delta;
	s_queue.push({-1, 0, x, y, 0, 0, delta});
}

} // namespace detail

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void setCursorPosition(int_t x, int_t y)
{
	detail::s_cursorX = x;
	detail::s_cursorY = y;
	// Deliberately different from the PS2/Wii setters: on an absolute touch
	// pointer a warp just means "the cursor is here now". Zero the staged
	// motion so a stale touch delta can't rotate the camera on the first
	// gameplay frames after a GUI warp, and queue NO motion event — the GUI
	// that warped us re-reads getX()/getY() itself.
	detail::s_stagingDX = 0;
	detail::s_stagingDY = 0;
}

bool next()
{
	if (detail::s_queue.empty()) return false;
	detail::s_current = detail::s_queue.front();
	detail::s_queue.pop();
	return true;
}

int_t getEventButton()      { return detail::s_current.button; }
bool  getEventButtonState() { return detail::s_current.down != 0; }
int_t getEventDX()          { return detail::s_current.xrel; }
int_t getEventDY()          { return -detail::s_current.yrel; }
int_t getEventX()           { return detail::s_current.x; }
int_t getEventY()           { return lwjgl::Display::getHeight() - detail::s_current.y - 1; }
int_t getEventDWheel()      { return detail::s_current.wheel; }

int_t getX() { return detail::s_cursorX; }
int_t getY() { return lwjgl::Display::getHeight() - detail::s_cursorY - 1; }

int_t getDX()
{
	int v = detail::s_stagingDX;
	detail::s_stagingDX = 0;
	return v;
}
int_t getDY()
{
	int v = detail::s_stagingDY;
	detail::s_stagingDY = 0;
	return v;
}
int_t getDWheel()
{
	int v = detail::s_stagingDW;
	detail::s_stagingDW = 0;
	return v;
}

void clearDeltas()
{
	detail::s_stagingDX = 0;
	detail::s_stagingDY = 0;
	detail::s_stagingDW = 0;
}

bool isButtonDown(int_t button)
{
	if (button < 0 || button > 2) return false;
	return detail::s_btnDown[button];
}

bool isGrabbed() { return detail::s_grabbed; }

void setGrabbed(bool grabbed)
{
	// Grab semantics for the touch pointer are Phase 2: a finger on the
	// resistive panel can't "leave the window" the way a desktop cursor can,
	// so for now the flag is simply stored and reported back by isGrabbed().
	detail::s_grabbed = grabbed;
}

} // namespace Mouse
} // namespace lwjgl

#endif // CTR_PLATFORM
