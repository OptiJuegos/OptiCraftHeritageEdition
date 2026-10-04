// Nintendo 3DS full-game entry point. libctru bring-up lives here (video,
// bottom-screen console, SD hand-off, CPU speedup); the mount itself is shared
// with the pre-main callers in DsBootstrap.cpp, input lifecycle in DsInput.
// Mirrors src/wii/main_wii.cpp: platform early-init -> local credentials ->
// Minecraft::start() -> teardown, with no loop of our own -- when the game
// returns we fall straight through to shutdown.
#ifdef CTR_PLATFORM

#include <3ds.h>
#include <citro3d.h>

#include <cstdio>

#include "platform/Log.h"
#include "platform/Thread.h"
#include "client/Minecraft.h"
#include "java/String.h"
#include "3ds/DsBootstrap.h"
#include "3ds/system/DsEarlyCrash.h"

// Main-thread stack size override: libctru's crt0 ships a weak 32 KiB
// __stacksize__, and that is not enough for this game's real call chains --
// minizip's zipOpen3 frames during the QR pack install alone overflowed it
// on hardware (Luma data abort, write below the mapped stack block,
// 2026-10-01). A strong definition here wins over the crt0 weak one. The
// CIA path cannot rely on the exheader's StackSize key for the same job
// because the .3dsx / Homebrew Launcher loader reads no exheader at all.
extern "C" std::uint32_t __stacksize__ = 0x80000;

namespace
{

// The bottom-screen console is this port's only voice before the game runs,
// so a startup failure has to stay on screen until the player has read it:
// exiting immediately would flash the instructions and hand back a blank
// Homebrew Launcher. Same contract as the Wii's missing-assets screen, which
// waits for HOME before it returns (WiiBootstrap.cpp).
//
// Only START dismisses: A/B/X/Y are deliberately not accepted, because the
// loader hands back to the Homebrew Launcher and those buttons are how it
// navigates -- swallowing a press here would eat the player's first input on
// return.
void waitForDismissal()
{
	std::printf("\nPress START to exit.\n");
	std::fflush(stdout);
	while (aptMainLoop())
	{
		hidScanInput();
		if (hidKeysDown() & KEY_START)
			break;
		gspWaitForVBlank();
	}
}

// Undo the bootstrap below. The order is gfxExit() then fsExit(): the two
// services are independent, so the ordering is cosmetic -- but fsExit() must
// pair the fsInit() taken inside dsEnsureStorage(), and libctru services are
// ref-counted, so a failed fsInit() makes this pair a no-op rather than an
// unbalance. consoleInit() has no teardown of its own in this libctru
// (there is no consoleExit() in 3ds/console.h); the console framebuffer dies
// with gfxExit().
//
// Before tearing down the GPU we must ensure all queued frames have finished
// executing. The normal frame loop paces via C3D_FrameBegin(SYNCDRAW) at the
// start of the *next* frame, but on shutdown there is no next frame. Without
// an explicit sync the last swapBuffers() may leave a frame in flight, and
// gfxExit() tearing down the context while the GPU is still reading the
// command buffer causes the intermittent "exit crash" observed on hardware.
void shutdownServices()
{
	// Flush any pending GPU work by doing a dummy SYNCDRAW frame.
	// C3D_FrameBegin(C3D_FRAME_SYNCDRAW) waits for the GX queue to drain.
	if (C3D_FrameBegin(C3D_FRAME_SYNCDRAW))
	{
		C3D_FrameEnd(0);
	}
	gfxExit();
	fsExit();
}

} // namespace

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	// Framebuffers first, then the text console -- but on the BOTTOM screen:
	// the top panel (400x240) is the game surface and must stay free of the
	// console from the very first line printed.
	//
	// The two panels now take opposite double-buffering settings, because
	// different halves own them:
	//
	//   * TOP: citro3d owns it. Every C3D_FrameEnd queues the display
	//     transfer into libctru's back buffer, and citro3d's queue-finished
	//     callback swaps the front/back pair once the transfer lands
	//     (renderqueue.c: onQueueFinish -> gfxScreenSwapBuffers). Double
	//     buffering must therefore be ON: with it off, the transfer would
	//     write the very buffer the LCD is scanning -- tearing on hardware,
	//     half-completed frames on a slow emulator.
	//
	//   * BOTTOM: the boot console owns it, and writes it directly through
	//     gfxGetFramebuffer(). With double buffering on, those writes land
	//     on a back buffer that nothing ever swaps in, so every printf would
	//     go off-screen and both panels would seem dead. (consoleInit()
	//     forces this itself for its screen; the explicit call keeps the
	//     asymmetry between the two panels readable at the boot site.)
	gfxInitDefault();
	gfxSetDoubleBuffering(GFX_TOP, true);
	gfxSetDoubleBuffering(GFX_BOTTOM, false);
	consoleInit(GFX_BOTTOM, nullptr);

	// Terminate handler before anything else can throw: an uncaught C++
	// exception otherwise dies through libstdc++'s default terminate, whose
	// one-line stderr note is the whole crash report and dies with the run.
	// The handler routes through CrashHandler::Crash() -- bottom screen held
	// until START, reason copied into the file log.
	DsEarlyCrash::install();

	// fsInit + the "sdmc:" mount + mkdir sdmc:/opticraft, all in DsBootstrap
	// because java::File objects and Resource lookups can trigger the same
	// mount from static initialisers before main(). Calling it here first
	// just moves a broken-SD failure to where we can still print why.
	if (!dsEnsureStorage())
	{
		std::printf("OptiCraft could not mount the SD card.\n\n");
		std::printf("Insert a writable SD card and relaunch.\n");
		waitForDismissal();
		shutdownServices();
		return 0;
	}

	// The sink is re-opened here as well as in dsEnsureStorage(). That earlier
	// call can run from a static initialiser (java::File, Resource -- see
	// DsBootstrap.cpp), before stdio and the bottom console exist, where
	// fopen() fails and nothing ever tries again; openSessionFile()
	// short-circuits only on success, so this retry is free when the early call
	// did work, and replays whatever the early buffer caught.
	//
	// The call itself stays unconditional, not under MC_LOG: CrashHandler_3ds
	// writes crash reports straight to McLog::write regardless of the build's
	// log level, so the sink must be open even in a silent release build.
	const bool logSinkOpen = McLog::openSessionFile(dsGetAppDir());
	MC_LOG_INFO("3ds", "log sink %s\n", logSinkOpen ? "open" : "failed");

	// The data tree is staged at bin/3ds/sd/opticraft by the 3ds-data target;
	// without it the game would die much later behind a missing-resource
	// exception with far less useful instructions.
	if (!dsHasGameData())
	{
		std::printf("OptiCraft could not find its game data.\n\n");
		std::printf("Copy the folder\n");
		std::printf("    bin/3ds/sd/opticraft/\n");
		std::printf("to the ROOT of your SD card, so the card contains:\n");
		std::printf("    sdmc:/opticraft/data/assets/...\n");
		std::printf("(or sdmc:/opticraft/assets.pak for a packed install)\n\n");
		std::printf("Stage it first with:\n");
		std::printf("    cmake --build <build-dir> --target 3ds-data\n");
		waitForDismissal();
		shutdownServices();
		return 0;
	}
	// Game data presence is a boot milestone, not a user-facing problem:
	// the missing case above already printed its instructions. MC_LOG_INFO
	// keeps it out of a silent release build (level 0), matching the
	// milestone logging the Wii main does.
	MC_LOG_INFO("3ds", "game data present\n");

	// New 3DS 804 MHz boost -- but ONLY when the console really is a New 3DS.
	//
	// The earlier "call it unconditionally, it is a no-op on Old hardware"
	// reading does not survive contact with an emulator: osSetSpeedupEnable()
	// sends 0x0818 to ptm:sysm and blocks waiting for the reply, so anything
	// that does not implement that command never returns from the call at all.
	// Azahar reports ConfigureNew3DSCPU as unimplemented and the boot stopped
	// dead on this exact line -- 0 FPS, both panels black, no further HLE
	// traffic anywhere in the log. APT_CheckNew3DS is the cheap, documented
	// way to skip a boost that could not apply anyway; it also fails safe,
	// because isNew3ds stays false and the boost is skipped.
	bool isNew3ds = false;
	const Result new3dsResult = APT_CheckNew3DS(&isNew3ds);
	const char *model = "unknown";
	if (R_SUCCEEDED(new3dsResult))
		model = isNew3ds ? "New 3DS" : "Old 3DS";
	if (isNew3ds)
		osSetSpeedupEnable(true);
	MC_LOG_INFO("3ds", "cpu: %s, boost %s\n", model, isNew3ds ? "on" : "off");

	// Ask the OS for time on the second ARM11. Old and New 3DS alike grant an
	// application a share of core 1 only after APT_SetAppCpuTimeLimit, and
	// without it a thread pinned to that core would never be scheduled at all
	// -- so Thread.cpp refuses the affinity until this call reports success,
	// and the async chunk-generation worker then runs beside the game thread
	// instead of time-slicing core 0 with it. 80 is the value every serious
	// 3DS port ships: the OS keeps its own slice, the game keeps a full core,
	// and the worker gets the rest. On a loader that refuses the request the
	// worker falls back to the default core, where the per-frame publish
	// budget still keeps generation off the frame path.
	const Result cpuLimitResult = APT_SetAppCpuTimeLimit(80);
	if (R_SUCCEEDED(cpuLimitResult))
	{
		PlatformThread::setSecondaryCoreAvailable(true);
		MC_LOG_INFO("3ds", "secondary core granted (cpu time limit 80%%)\n");
	}
	else
		MC_LOG_WARN("3ds", "APT_SetAppCpuTimeLimit failed (%08lx); workers share the main core\n",
		             cpuLimitResult);

	MC_LOG_INFO("3ds", "handing off to Minecraft::start()\n");
	jstring username = "Player";
	jstring auth = "-";
	Minecraft::start(&username, &auth);
	MC_LOG_INFO("3ds", "Minecraft::start returned; exiting to loader\n");

	// The game's frame loop ended (close requested via Display, a handled
	// error, ...). Nothing here spins: fall through to shutdown.
	shutdownServices();
	return 0;
}

#endif // CTR_PLATFORM
