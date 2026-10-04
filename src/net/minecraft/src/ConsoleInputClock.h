#pragma once

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM) || defined(CTR_PLATFORM)

#if defined(PS2_PLATFORM)
#include "platform/time.h"
#elif defined(WII_PLATFORM)
#include <ogc/lwp_watchdog.h>
#else
#include <3ds.h>
#endif

// Millisecond clock for the console input helpers -- the on-screen keyboard and
// the container slot navigation. Both run from the pad poll rather than from a
// world tick, so neither can count ticks, and both need auto-repeat timing while
// a GUI is open.
//
// Not System::currentTimeMillis(): that rides std::chrono::system_clock, which
// on the PS2 is only as alive as the BIOS timer behind it -- the same source
// that leaves System::nanoTime() sitting still on some revisions. These are the
// clocks each console's own code already paces itself with.
inline int consoleInputNowMs()
{
#if defined(PS2_PLATFORM)
	return (int)(getTimeS() * 1000.0f);
#elif defined(WII_PLATFORM)
	return (int)ticks_to_millisecs(gettime());
#else
	// The 268 MHz system tick, not osGetTime(): this clock only needs
	// millisecond resolution for auto-repeat, and the tick counter stays at
	// 268 MHz even with the New 3DS core boosted to 804 MHz (libctru's
	// SYSCLOCK_ARM11 is documented as the rate "in CTR mode and in
	// svcGetSystemTick"), so one code path serves both models.
	//
	// Masked to 30 bits so the result stays a positive int and the
	// "now + repeat delay" additions below cannot overflow INT_MAX; the mask
	// wraps about every 12.4 days, costing at most one missed auto-repeat.
	return (int)((svcGetSystemTick() / (u64)CPU_TICKS_PER_MSEC) & 0x3FFFFFFF);
#endif
}

#endif // PS2_PLATFORM || WII_PLATFORM || CTR_PLATFORM
