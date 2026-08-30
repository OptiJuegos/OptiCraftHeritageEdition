#pragma once
#ifdef PS2_PLATFORM

namespace Ps2PadRuntime {
void initialize();
bool waitUntilReady();
void poll();
}

#endif
