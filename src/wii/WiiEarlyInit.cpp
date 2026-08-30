#ifdef WII_PLATFORM

#include "wii/WiiEarlyInit.h"
#include "wii/system/WiiEarlyCrash.h"
#include "wii/system/WiiEarlyMemory.h"

// Some translated Minecraft globals still load font.txt during static
// initialization. Keep this constructor until those resource reads are moved
// behind explicit game startup.
__attribute__((constructor(101)))
static void wiiEarlyInit()
{
	WiiEarlyCrash::install();
	wiiEnsureEarlyVideo();
	WiiEarlyMemory::captureInitialState();
	wiiEnsureStorage();
}

#endif // WII_PLATFORM
