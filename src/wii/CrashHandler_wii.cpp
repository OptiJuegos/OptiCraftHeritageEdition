// CrashHandler_wii.cpp — Wii implementation of CrashHandler::Crash().
//
// Providing this here means src/client/Minecraft.cpp needs no Wii branch: its
// existing `#ifndef PS2_PLATFORM` path calls straight into this.
//
// It does more than the PS2 equivalent. On a console a
// printf goes to a USB Gecko or Dolphin's log and nowhere else, so on real
// hardware a crash is a black screen and no information at all. This tears the
// GX pipeline down and brings libogc's text console back up on a fresh
// framebuffer, so the message lands on the TV where the player can read it --
// and, more usefully, photograph it and send it on.
#ifdef WII_PLATFORM

#include "wii/system/WiiConsole.h"
#include "pc/CrashHandler.h"

#include <cstdio>
#include <cstdlib>

#include <gccore.h>
#include "wii/WiiEarlyInit.h"
#include "wii/input/WiiInput.h"

namespace CrashHandler
{

void Crash(const std::string &message, const std::string &stackTrace)
{
	// Always emit to stdout first: if anything below fails, this is still the
	// record, and it is what Dolphin's log shows.
	WiiConsole::write("[WII] CRASH: %s\n", message.c_str());
	if (!stackTrace.empty())
		WiiConsole::write("[WII] trace:\n%s\n", stackTrace.c_str());

	// GX may be halfway through a frame holding the FIFO. Drop it before
	// touching video, or the console text fights whatever the GP still draws.
	GX_AbortFrame();

	// Reuse the framebuffer WiiEarlyInit already owns rather than allocating.
	// Out of memory is a plausible reason to be here in the first place, and a
	// crash handler that itself needs a successful allocation is a crash handler
	// that fails exactly when it is most needed.
	GXRModeObj *rmode = wiiGetRenderMode();
	void *xfb = wiiGetEarlyFramebuffer();

	if (rmode != nullptr && xfb != nullptr)
	{
		console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight,
		             rmode->fbWidth * VI_DISPLAY_PIX_SZ);
		VIDEO_Configure(rmode);
		VIDEO_SetNextFramebuffer(xfb);
		VIDEO_SetBlack(FALSE);
		VIDEO_Flush();
		VIDEO_WaitVSync();
		if (rmode->viTVMode & VI_NON_INTERLACE)
			VIDEO_WaitVSync();

		// Deliberately NOT clearing the screen: the boot trail printed above
		// (video, storage, card layout) is usually what explains the crash, and
		// wiping it throws away the context just when it is needed.
		WiiConsole::write("\n");
		WiiConsole::write("BetaPlusPlus has crashed\n");
		WiiConsole::write("------------------------\n\n");
		WiiConsole::write("%s\n", message.c_str());
		if (!stackTrace.empty())
			WiiConsole::write("\n%s\n", stackTrace.c_str());
		WiiConsole::write("\nPress HOME to return to the Homebrew Channel.\n");
	}

	WiiInput::waitForHome();

	std::exit(1);
}

} // namespace CrashHandler

#endif // WII_PLATFORM
