#pragma once

#ifdef PS2_PLATFORM

namespace Ps2Iop
{
    enum class RomModule
    {
        Sio2,
        Pad,
        MemoryCardManager,
        MemoryCardServer
    };

    void initFileServices();
    int loadModule(const char* path);
    int ensureRomModule(RomModule module);
}

#endif // PS2_PLATFORM
