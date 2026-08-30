// Ps2Clock.cpp — 64-bit extension of the EE cycle counter. See Ps2Clock.h.

#ifdef PS2_PLATFORM

#include "ps2/system/Ps2Clock.h"

namespace
{
	// EE core clock, in cycles per 1000 microseconds. 294912000 Hz is the
	// Emotion Engine's fixed core clock, so this is a hardware constant and not
	// a tuning value: cycles / 294912 gives milliseconds, and the arithmetic
	// below works in that unit to stay inside 32-bit division.
	const unsigned int kCyclesPerMilli = 294912u;

	// Deliberately file-static rather than a member of anything: there is one
	// COP0 Count and therefore one extension of it. Single-threaded by contract
	// (see the header).
	unsigned int       s_lastCycles = 0u;
	unsigned int       s_carry      = 0u; // sub-microsecond remainder, scaled x1000
	unsigned long long s_micros     = 0ull;
	bool               s_started    = false;
}

extern "C" unsigned long long ps2_ee_micros(void)
{
	const unsigned int now = ps2_ee_cycles();

	if (!s_started)
	{
		s_started = true;
		s_lastCycles = now;
		return 0ull;
	}

	// Unsigned subtraction is already correct across a single wrap.
	const unsigned int delta = now - s_lastCycles;
	s_lastCycles = now;

	// delta cycles -> microseconds, exactly, using only 32-bit division by a
	// constant. A single `delta * 1000 / 294912` would be the obvious form, but
	// delta * 1000 overflows 32 bits and a 64-bit divide on this core calls
	// __udivdi3 -- which would put back the cost this whole file exists to
	// remove. Splitting into whole milliseconds plus a remainder keeps every
	// division 32-bit, where GCC turns it into a multiply and a shift.
	const unsigned int whole  = delta / kCyclesPerMilli; // whole milliseconds
	const unsigned int rest   = delta % kCyclesPerMilli; // cycles left over
	const unsigned int scaled = rest * 1000u + s_carry;  // max ~295.2e6, fits

	s_micros += (unsigned long long)whole * 1000ull + (scaled / kCyclesPerMilli);
	s_carry = scaled % kCyclesPerMilli;

	return s_micros;
}

extern "C" long long ps2_ee_nanos(void)
{
	return (long long)(ps2_ee_micros() * 1000ull);
}

#endif // PS2_PLATFORM
