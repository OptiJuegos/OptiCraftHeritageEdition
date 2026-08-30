// Mouse_ps2.cpp — PS2 implementation of lwjgl::Mouse.
// Event queue fed by pad-poll helpers via pushMotion / pushButton / pushWheel.
#ifdef PS2_PLATFORM

#include "lwjgl/Mouse.h"
#include "lwjgl/Display.h"

#include <queue>
#include <tamtypes.h>

namespace lwjgl
{
namespace Mouse
{

// ---------------------------------------------------------------------------
// Internal event record (mirrors the PC version's struct)
// ---------------------------------------------------------------------------
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

static Event            s_current = {};
static std::queue<Event> s_queue;
static int              s_stagingDX = 0;
static int              s_stagingDY = 0;
static int              s_stagingDW = 0;
static int              s_cursorX   = 0;
static int              s_cursorY   = 0;
static bool             s_grabbed   = false;
static bool             s_btnDown[3] = {};  // left(0), right(1), middle(2)

void pushMotion(int x, int y, int xrel, int yrel)
{
    // Incoming PS2 cursor coordinates are top-left origin. LWJGL event/state
    // Y coordinates are bottom-left origin, so store top-left but expose
    // converted values through getEventY()/getY().
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

bool isGrabbed()            { return detail::s_grabbed; }
void setGrabbed(bool grabbed) { detail::s_grabbed = grabbed; }

} // namespace Mouse
} // namespace lwjgl

#endif // PS2_PLATFORM
