#ifdef WII_PLATFORM

#include "platform/Log.h"
#include "wii/WiiEarlyInit.h"

#include <gccore.h>

namespace
{
bool g_videoReady = false;
GXRModeObj* g_renderMode = nullptr;
void* g_framebuffer = nullptr;
}

void wiiEnsureEarlyVideo()
{
	if (g_videoReady)
		return;
	g_videoReady = true;

	VIDEO_Init();
	g_renderMode = VIDEO_GetPreferredMode(nullptr);
	g_framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(g_renderMode));
	console_init(g_framebuffer, 20, 20, g_renderMode->fbWidth, g_renderMode->xfbHeight,
	             g_renderMode->fbWidth * VI_DISPLAY_PIX_SZ);

	VIDEO_Configure(g_renderMode);
	VIDEO_SetNextFramebuffer(g_framebuffer);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (g_renderMode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();

	MC_LOG_INFO("wii", "BetaPlusPlus - Wii\n");
	MC_LOG_INFO("wii", "video %dx%d ok\n", g_renderMode->fbWidth, g_renderMode->xfbHeight);
}

GXRModeObj* wiiGetRenderMode()
{
	wiiEnsureEarlyVideo();
	return g_renderMode;
}

void* wiiGetEarlyFramebuffer()
{
	wiiEnsureEarlyVideo();
	return g_framebuffer;
}

#endif // WII_PLATFORM
