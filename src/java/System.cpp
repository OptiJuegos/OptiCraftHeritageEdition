#include "System.h"

#include <chrono>
#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM) && !defined(CTR_PLATFORM)
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
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM) || defined(CTR_PLATFORM)
	// No browser to hand the URL to on any of the three: the 3DS has the HOME
	// browser, but libctru exposes no supported way to hand it a URL.
	(void)url;
	return false;
#else
	return SDL_OpenURL(url.c_str()) == 0;
#endif
}

}
