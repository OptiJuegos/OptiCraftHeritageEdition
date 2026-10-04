// CrashHandler_3ds.cpp -- 3DS implementation of CrashHandler::Crash().
//
// Mirrors src/wii/CrashHandler_wii.cpp: on a console a crash must become text
// the player can read (and photograph), not a frozen frame. The 3DS has an
// easier job than the Wii here -- printf already reaches the bottom-screen
// console set up by main_3ds.cpp, with no framebuffer/console bring-up of our
// own -- so all this file adds is the hold (nothing may tear the console down
// while the message is being read) and the deterministic exit.
//
// Providing this here means src/client/Minecraft.cpp needs no 3DS branch: its
// existing call path goes straight into this.
#ifdef CTR_PLATFORM

#include "pc/CrashHandler.h"

#include <3ds.h>
#include <citro3d.h>

#include <cstdio>
#include <cstdlib>

#include "platform/Log.h"
#include "3ds/render/DsRender.h"

namespace
{

// Same role as WiiInput::waitForHome() in the Wii version: the message is the
// entire point of a crash screen, so hold it until the player dismisses it.
// START mirrors the bring-up smoke test's exit key (DsBringup.cpp); HOME and
// sleep are handled by aptMainLoop() either way.
void waitForDismissal()
{
	while (aptMainLoop())
	{
		hidScanInput();
		if (hidKeysDown() & KEY_START)
			break;
		gspWaitForVBlank();
	}
}

} // namespace

namespace CrashHandler
{

void Crash(const std::string &message, const std::string &stackTrace)
{
	// The dual-screen GUI may have taken the bottom LCD over at BGR8
	// (DsRender.cpp), while the console renders RGB565 text -- printing
	// straight away would write the report in the wrong pixel format over
	// the panel image. Hand the screen back first: blank the panel (scanned
	// as RGB565 it is garbage), re-present so the LCD picks the format up,
	// then print. Before the panel exists nothing needs doing -- the format
	// is still consoleInit()'s RGB565 and the boot log stays on screen.
	if (ds::bottomPanelOwned())
	{
		gfxSetScreenFormat(GFX_BOTTOM, GSP_RGB565_OES);
		consoleClear();
		gfxScreenSwapBuffers(GFX_BOTTOM, false);
		gspWaitForVBlank();
	}

	// Print first, always: stdout is unbuffered after consoleInit(), so this
	// lands on the bottom screen even if everything below misbehaves.
	std::printf("\nOptiCraft has crashed\n");
	std::printf("---------------------\n\n");
	std::printf("%s\n", message.c_str());
	if (!stackTrace.empty())
		std::printf("\n%s\n", stackTrace.c_str());
	std::printf("\nPress START to exit.\n");
	// Belt and braces: a crash may have upset the stdio state the console set
	// up, and an unflushed tail is a crash report that stops early.
	std::fflush(stdout);

	// The file log is the copy that survives the run: a report that only
	// exists on the bottom screen is one the player has to transcribe by
	// hand. Write straight through, regardless of the build's log level --
	// a crash report must land in debug.log the same way the screen holds
	// this one.
	McLog::write(McLog::Level::Error, "crash", "%s", message.c_str());
	if (!stackTrace.empty())
		McLog::write(McLog::Level::Error, "crash", "%s", stackTrace.c_str());
	McLog::flush();

	waitForDismissal();

	// Same teardown order and exit code as CrashHandler_wii.cpp (std::exit(1)
	// after the services are down); the service order matches
	// main_3ds.cpp's shutdownServices().
	// Flush any pending GPU work before tearing down the context (see
	// shutdownServices() in main_3ds.cpp for the same rationale).
	if (C3D_FrameBegin(C3D_FRAME_SYNCDRAW))
	{
		C3D_FrameEnd(0);
	}
	gfxExit();
	fsExit();
	std::exit(1);
}

} // namespace CrashHandler

#endif // CTR_PLATFORM
