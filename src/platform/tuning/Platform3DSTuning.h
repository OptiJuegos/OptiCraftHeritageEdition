#pragma once

// -----------------------------------------------------------------------------
// 3DS overrides
// -----------------------------------------------------------------------------
// Same idea as the Wii file below it: the 3DS takes the desktop branch of the
// tuning table and overrides the desktop assumptions the ARM11 cannot honour
// -- kept as an override file rather than a third full copy of the table, for
// the same drift reasons PlatformWiiTuning.h states. Every value, and the
// debug.log measurements that produced it, lives in 3ds/tuning/DsWorldTuning.h.
#if PLATFORM_3DS
#  include "3ds/tuning/DsWorldTuning.h"
#endif
