#pragma once
#ifdef WII_PLATFORM

#include <gccore.h>

namespace WiiPadInternal { struct FrameState; }

namespace WiiGameCubePad
{
void poll(u32 connectedMask, WiiPadInternal::FrameState& state);
}

#endif // WII_PLATFORM
