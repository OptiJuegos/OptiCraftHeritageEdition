// Display_wii.cpp — Wii implementation of lwjgl::Display.
//
// Thin wrapper: gx_wii.cpp owns VI, the framebuffers and the flip (see the
// comment on wiigl_init in gx_wii.h for why that sequence is not split up).
// What lives here is the lwjgl-shaped surface plus the two things that are
// genuinely display-level on this console:
//
//   * "Close requested" has no window to close. It maps to the console's own
//     exit affordances -- the RESET and POWER buttons on the front panel, and
//     HOME on a Wiimote -- which arrive as libogc callbacks rather than events.
//     Handling them matters: homebrew that ignores RESET/POWER feels broken,
//     and an unsaved world dies with it.
//
//   * The video mode is not ours to choose. VIDEO_GetPreferredMode reports what
//     the console and the user's cable are set to (NTSC 640x480, PAL 640x528,
//     progressive if a component cable is attached), so setDisplayMode is
//     advisory only.
#ifdef WII_PLATFORM

#include "lwjgl/Display.h"

#include <gccore.h>

#include "wii/gx_wii.h"
#include "wii/input/WiiInput.h"
#include "wii/system/WiiSystemEvents.h"

namespace
{
bool g_created = false;
}

namespace lwjgl
{
namespace Display
{

void create()
{
	if (g_created) return;

#ifdef WII_WIDESCREEN
	wiigl_init(true);
#else
	wiigl_init(false);
#endif

	WiiInput::initialize(wiigl_width(), wiigl_height());
	WiiSystemEvents::install();

	g_created = true;
}

void setDisplayMode(const DisplayMode &)
{
	// The console decides. See the header comment.
}

DisplayMode getDisplayMode()
{
	return DisplayMode(wiigl_width(), wiigl_height());
}

void setTitle(const jstring &) {}
void setFullscreen(bool)       {}

bool isCloseRequested() { return WiiSystemEvents::exitRequested(); }
bool isVisible()        { return true; }
bool isActive()         { return true; }

void processMessages()
{
	WiiInput::poll();
}

void swapBuffers()
{
	wiigl_end_frame();
	wiigl_begin_frame();
}

void update(bool doProcessMessages)
{
	swapBuffers();
	if (doProcessMessages)
		processMessages();
}

int_t getX() { return 0; }
int_t getY() { return 0; }
int_t getWidth()  { return wiigl_width(); }
int_t getHeight() { return wiigl_height(); }

} // namespace Display
} // namespace lwjgl

#endif // WII_PLATFORM
