#pragma once

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)

#if defined(PS2_PLATFORM)
#include "platform/time.h"
#else
#include <ogc/lwp_watchdog.h>
#endif

// Millisecond clock for the console input helpers -- the on-screen keyboard and
// the container slot navigation. Both run from the pad poll rather than from a
// world tick, so neither can count ticks, and both need auto-repeat timing while
// a GUI is open.
//
// Not System::currentTimeMillis(): that rides std::chrono::system_clock, which
// on the PS2 is only as alive as the BIOS timer behind it -- the same source
// that leaves System::nanoTime() sitting still on some revisions. These two are
// the clocks each console's own code already paces itself with.
inline int consoleInputNowMs()
{
#if defined(PS2_PLATFORM)
	return (int)(getTimeS() * 1000.0f);
#else
	return (int)ticks_to_millisecs(gettime());
#endif
}

#endif // PS2_PLATFORM || WII_PLATFORM
