#include "Timer.h"

#include "java/Arithmetic.h"
#include "java/System.h"

Timer::Timer(float f) :
	ticksPerSecond(f),
	elapsedTicks(0),
	renderPartialTicks(0.0f),
	timerSpeed(1.0f),
	elapsedPartialTicks(0.0f),
	lastHRTime(0.0),
	lastSyncSysClock(System::currentTimeMillis()),
	lastSyncHRClock(System::nanoTime() / 0xf4240LL),
	accumulatedSysClock(0),
	timeSyncAdjustment(1.0)
{
}

void Timer::updateTimer()
{
	long_t l = System::currentTimeMillis();
	long_t l1 = JavaArithmetic::longSub(l, lastSyncSysClock);
	long_t l2 = System::nanoTime() / 0xf4240LL;
	double d = (double)l2 / 1000.0;
	if (l1 > 1000LL)
	{
		lastHRTime = d;
	}
	else if (l1 < 0LL)
	{
		lastHRTime = d;
	}
	else
	{
		accumulatedSysClock = JavaArithmetic::longAdd(accumulatedSysClock, l1);
		if (accumulatedSysClock > 1000LL)
		{
			long_t l3 = JavaArithmetic::longSub(l2, lastSyncHRClock);
			double d2 = (double)accumulatedSysClock / (double)l3;
			timeSyncAdjustment += (d2 - timeSyncAdjustment) * 0.20000000298023224;
			lastSyncHRClock = l2;
			accumulatedSysClock = 0LL;
		}
		if (accumulatedSysClock < 0LL)
		{
			lastSyncHRClock = l2;
		}
	}
	lastSyncSysClock = l;
	double d1 = (d - lastHRTime) * timeSyncAdjustment;
	lastHRTime = d;
	if (d1 < 0.0)
	{
		d1 = 0.0;
	}
	if (d1 > 1.0)
	{
		d1 = 1.0;
	}
	elapsedPartialTicks += d1 * (double)timerSpeed * (double)ticksPerSecond;
	elapsedTicks = JavaArithmetic::floatToInt(elapsedPartialTicks);
	elapsedPartialTicks -= elapsedTicks;
#if defined(PS2_PLATFORM)
	// Vanilla can try to catch up by running up to 10 game ticks in one rendered
	// frame. On PS2 that creates a death spiral while chunks/worldgen are slow:
	// one long frame queues 10 expensive ticks, those ticks make the next frame
	// even longer, and the player sees multi-second freezes. Prefer temporary
	// slow-motion over unbounded catch-up stalls.
	if (elapsedTicks > 1)
	{
		elapsedTicks = 1;
	}
#elif defined(WII_PLATFORM)
	// Bound catch-up work so one slow chunk or mesh frame cannot queue enough
	// simulation work to cause a self-sustaining sequence of long frames.
	if (elapsedTicks > 2)
	{
		elapsedTicks = 2;
	}
#else
	if (elapsedTicks > 10)
	{
		elapsedTicks = 10;
	}
#endif
	renderPartialTicks = elapsedPartialTicks;
}
