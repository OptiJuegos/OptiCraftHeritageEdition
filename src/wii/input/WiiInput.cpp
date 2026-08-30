#ifdef WII_PLATFORM
#include "wii/input/WiiInput.h"
#include "wii/input/WiiPadPoll.h"

#include <cstdint>
#include <gccore.h>
#include <wiiuse/wpad.h>

namespace
{
bool g_initialized = false;
int g_pointerWidth = 640;
int g_pointerHeight = 480;
}

namespace WiiInput
{

bool initialize(int pointerWidth, int pointerHeight)
{
	if (!g_initialized)
	{
		WPAD_Init();
		PAD_Init();
		WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
		g_initialized = true;
	}
	setPointerResolution(pointerWidth, pointerHeight);
	return true;
}

void setPointerResolution(int width, int height)
{
	if (width > 0) g_pointerWidth = width;
	if (height > 0) g_pointerHeight = height;
	if (g_initialized)
		WPAD_SetVRes(WPAD_CHAN_ALL, g_pointerWidth, g_pointerHeight);
}

void poll()
{
	if (!g_initialized)
		initialize(g_pointerWidth, g_pointerHeight);
	const std::uint32_t gameCubeConnected = static_cast<std::uint32_t>(PAD_ScanPads());
	WPAD_ScanPads();
	wiiPadPoll(gameCubeConnected);
}

void shutdown()
{
	if (!g_initialized) return;
	WPAD_Shutdown();
	g_initialized = false;
}

void waitForHome()
{
	if (!g_initialized)
		initialize(g_pointerWidth, g_pointerHeight);
	while (true)
	{
		WPAD_ScanPads();
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
			return;
		VIDEO_WaitVSync();
	}
}

bool initialized()
{
	return g_initialized;
}

}
#endif
