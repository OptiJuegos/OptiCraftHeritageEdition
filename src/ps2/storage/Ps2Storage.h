#pragma once

#ifdef PS2_PLATFORM

#include <cstddef>
#include <string>

namespace Ps2Storage
{
    // Marks IOP/newlib file services as initialized. Asset probing can happen
    // from static initializers, so callers must not wait for USB before this.
    void setFileIoReady();
    bool fileIoReady();

    bool massAvailable();
    std::string massRoot();

    // Reads a whole file into an aligned allocation for DMA-oriented consumers.
    // Returns nullptr on error; the caller owns a successful result and free()s it.
    unsigned char* readWholeFile(const std::string& path,
                                 unsigned int* outSize,
                                 std::size_t alignment = 64);
}

#endif // PS2_PLATFORM
