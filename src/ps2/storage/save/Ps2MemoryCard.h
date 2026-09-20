#pragma once

#ifdef PS2_PLATFORM

namespace Ps2MemoryCard
{
    bool initialize();
    bool available(); // Read-only, re-probes hot-plug state; never formats.
    bool probeWritable();
    bool isFormatted();
}

#endif // PS2_PLATFORM
