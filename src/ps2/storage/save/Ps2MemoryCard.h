#pragma once

#ifdef PS2_PLATFORM

namespace Ps2MemoryCard
{
    bool initialize();
    bool probeWritable();
    bool isFormatted();
}

#endif // PS2_PLATFORM
