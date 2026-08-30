// Ps2Clock.h — cheap monotonic clock for the Emotion Engine.
//
// What this replaces
// ------------------
// Everything that measures elapsed time on this console used to go through
// std::chrono, and on ps2sdk that lands in newlib. Traced through the linked
// ELF, one System::nanoTime() is five calls deep:
//
//   System::nanoTime -> chrono::system_clock::now -> gettimeofday
//                    -> _gettimeofday_r -> (three more)
//
// which is an absurd price for a number used to decide whether a per-frame
// budget has run out -- the measurement can cost a noticeable slice of what it
// is measuring. The R5900 answers the same question in ONE instruction: COP0
// register 9 (Count) increments once per CPU cycle.
//
// What it does NOT replace
// ------------------------
// System::nanoTime() itself stays on std::chrono, deliberately. Timer.cpp uses
// it as the game clock and cross-calibrates it against currentTimeMillis()
// through timeSyncAdjustment, so the simulation rate rides on it directly. This
// clock is for measuring durations, not for pacing the world.
//
// Wrap-around and why there is state
// ----------------------------------
// Count is 32 bits at 294.912 MHz, so it wraps every ~14.6 seconds. Callers
// subtract absolute values (now - start), which a wrapping counter would turn
// into a nonsense delta, so the accumulator below extends it to 64 bits by
// summing deltas. That has one requirement: it must be read at least once per
// wrap period. Every current caller runs per frame, so this holds during play;
// a gap longer than ~14.6s between two calls loses one wrap and reports that
// interval as shorter than it was. No caller cares -- they all measure
// milliseconds inside a single frame.
//
// Not thread-safe, and does not need to be: only the main thread measures
// anything. src/ps2/input/Ps2Input.cpp starts one EE thread for pad init, which
// never calls this.
#pragma once

#ifdef PS2_PLATFORM

#ifdef __cplusplus
extern "C" {
#endif

// Raw COP0 Count: EE core cycles, wrapping. One instruction, no call.
static inline unsigned int ps2_ee_cycles(void)
{
	unsigned int c;
	__asm__ __volatile__("mfc0 %0, $9" : "=r"(c));
	return c;
}

// Monotonic microseconds since the first call. Extends ps2_ee_cycles() to 64
// bits; see the wrap note above.
unsigned long long ps2_ee_micros(void);

// Same clock in nanoseconds, for the perf counters that report in ns. The
// resolution is still one microsecond -- this only rescales -- which is three
// orders of magnitude finer than the milliseconds those counters print.
long long ps2_ee_nanos(void);

#ifdef __cplusplus
}
#endif

#endif // PS2_PLATFORM
