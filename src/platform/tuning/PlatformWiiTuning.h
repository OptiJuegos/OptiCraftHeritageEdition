#pragma once

// -----------------------------------------------------------------------------
// Wii overrides
// -----------------------------------------------------------------------------
// The Wii takes the desktop branch above on purpose -- PLATFORM_CONSOLE_LOW is 0
// for it (see PlatformConfig.h) -- and then overrides the handful of values that
// are desktop *assumptions* rather than deliberate choices. Kept as an override
// file rather than a third full copy of the table: the Wii agrees with the
// desktop baseline on roughly fifty of these knobs, and a copy would drift.
#if PLATFORM_WII
#  include "wii/WiiTuning.h"
#endif
