#ifdef WII_PLATFORM
#include "platform/Log.h"
#include "wii/system/WiiBootstrap.h"

#include <string>

#include "lwjgl/Display.h"
#include "pc/CrashHandler.h"
#include "wii/WiiEarlyInit.h"
#include "wii/gx_wii.h"
#include "wii/input/WiiInput.h"

namespace WiiBootstrap
{

bool initialize()
{
	MC_LOG_INFO("wii", "main() entered\n");

	if (!wiiEnsureStorage())
	{
		CrashHandler::Crash(
			"No SD or USB storage could be mounted.\n\n"
			"BetaPlusPlus reads its assets and saves its worlds under\n"
			"apps/BetaPlusPlus/ on an SD card or a USB drive.");
		return false;
	}

	if (!wiiHasGameData())
	{
		CrashHandler::Crash(
			std::string("No game data found.\n\nLooked in: ") + wiiGetAppDir() +
			"/data/assets\n\n"
			"Dolphin:\n"
			"1. Open Config > Wii > SD Card.\n"
			"2. Enable 'Automatically Sync with Folder'.\n"
			"3. Select the folder containing apps/BetaPlusPlus.\n"
			"4. Restart emulation.\n\n"
			"Expected: apps/BetaPlusPlus/data/assets\n"
			"Real Wii: copy the complete BetaPlusPlus app folder again.");
		return false;
	}

	MC_LOG_INFO("wii", "app dir: %s\n", wiiGetAppDir());
	MC_LOG_INFO("wii", "starting GX/input...\n");
	lwjgl::Display::create();
	return true;
}

void shutdown()
{
	WiiInput::shutdown();
	wiigl_shutdown();
}

}
#endif
