#pragma once

#include "java/Type.h"

// net.minecraft.src.Timer
class Timer
{
public:
	Timer(float f);

	void updateTimer();

	float ticksPerSecond;
	int_t elapsedTicks;
	float renderPartialTicks;
	float timerSpeed;
	float elapsedPartialTicks;

private:
	double lastHRTime;
	long_t lastSyncSysClock;
	long_t lastSyncHRClock;
	long_t accumulatedSysClock;
	double timeSyncAdjustment;
};
