#pragma once

#include "java/Type.h"

#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
#include "SDL_events.h"
#endif

namespace lwjgl
{
namespace Mouse
{
namespace detail
{

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
// Consoles have no SDL event pump; the platform's input poll feeds events here.
//   PS2 — right stick drives a simulated cursor.
//   Wii — the Wiimote IR pointer drives it directly, with the Classic/GC right
//         stick as the fallback when the pointer is off-screen.
void pushMotion(int x, int y, int xrel, int yrel);
void pushButton(int button, bool down, int x, int y);
void pushWheel(int delta, int x, int y);
#else
void pushEvent(const SDL_Event &e);
#endif

}

void setCursorPosition(int_t x, int_t y);

// Event handling
bool next();

int_t getEventButton();
bool getEventButtonState();

int_t getEventDX();
int_t getEventDY();

int_t getEventX();
int_t getEventY();

int_t getEventDWheel();

// State
int_t getX();
int_t getY();

int_t getDX();
int_t getDY();

int_t getDWheel();

// Clears accumulated relative mouse motion and wheel deltas.
// Useful after grabbing/ungrabbing the mouse so stale menu or warp deltas
// do not rotate the camera on the first gameplay frames.
void clearDeltas();

bool isButtonDown(int_t button);

bool isGrabbed();
void setGrabbed(bool grabbed);

}
}
