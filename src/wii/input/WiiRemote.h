#pragma once
#ifdef WII_PLATFORM

namespace WiiPadInternal { struct FrameState; }

namespace WiiRemote
{
void poll(WiiPadInternal::FrameState& state);
}

#endif // WII_PLATFORM
