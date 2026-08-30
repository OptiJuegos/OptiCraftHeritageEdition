#pragma once

#ifdef WII_PLATFORM

namespace WiiPointer
{
// Starts a new controller frame, measures a clamped monotonic delta, and clears
// the per-frame look accumulators while preserving cursor/sub-pixel state.
void beginFrame();

// Adds normalized stick look input. Multiple connected controllers contribute
// to the same vector and are clamped once when the frame is flushed.
void addStickLook(float x, float y);

// Processes one bounded IR coordinate. In gameplay it is interpreted as a
// continuous turn rate; in GUIs it remains an absolute cursor. Returns the
// radial deflection percentage used by the input diagnostic overlay.
int handleIr(int x, int y, bool allowGameplayLook);
void loseIr();

// Converts the accumulated stick/IR input into lwjgl mouse motion.
void flush();

// Absolute GUI cursor placement used by shared container navigation.
void setCursorPosition(int x, int y);
int cursorX();
int cursorY();
}

#endif
