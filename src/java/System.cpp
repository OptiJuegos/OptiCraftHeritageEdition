#include "System.h"

#include <chrono>
#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
#include <SDL.h>
#endif

namespace System
{

long_t currentTimeMillis()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

long_t nanoTime()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

bool openURL(const std::string &url)
{
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
	// No browser to hand the URL to on either console.
	(void)url;
	return false;
#else
	return SDL_OpenURL(url.c_str()) == 0;
#endif
}

}
