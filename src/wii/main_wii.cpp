// Nintendo Wii full-game entry point. Hardware bring-up lives in WiiBootstrap;
// controller lifecycle in WiiInput; game code only starts after both are ready.
#ifdef WII_PLATFORM

#include "platform/Log.h"
#include "client/Minecraft.h"
#include "java/String.h"
#include "wii/WiiEarlyInit.h"
#include "wii/system/WiiBootstrap.h"

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	if (!WiiBootstrap::initialize())
		return 1;

	MC_LOG_INFO("wii", "handing off to Minecraft::start()\n");
	jstring username = "Player";
	jstring auth = "-";
	Minecraft::start(&username, &auth);
	MC_LOG_INFO("wii", "Minecraft::start returned; exiting to loader\n");

	WiiBootstrap::shutdown();
	return 0;
}

#endif
