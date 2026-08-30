#pragma once
#ifdef WII_PLATFORM

namespace WiiInput
{
// Owns libogc controller lifecycle. Safe to call more than once.
bool initialize(int pointerWidth, int pointerHeight);
void setPointerResolution(int width, int height);
void poll();
void shutdown();

// Crash-safe path: makes the Wiimote stack usable even if normal display/input
// initialization never completed, then blocks until HOME is pressed.
void waitForHome();

bool initialized();
}

#endif
