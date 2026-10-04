#pragma once

#include <memory>
#include <vector>

#include "java/Type.h"
#include "platform/PlatformConfig.h"

// net.minecraft.src.IntCache
class IntCache
{
public:
    using Array = std::vector<int_t>;

    static Array &getIntCache(int_t requestedSize);
    static int_t checkedAreaSize(int_t width, int_t height);
    static void resetIntCache();

#if PLATFORM_WII || PLATFORM_3DS
    // Claims the second storage slot for the calling thread, which the async
    // chunk generator does for its worker. See IntCache.cpp for why the Wii
    // cannot express this as thread_local. The 3DS takes the same two-slot
    // scheme: devkitARM's thread_local is not trusted for this path either,
    // and there is exactly one generating thread to slot in.
    static void bindGenerationThread();
    static void unbindGenerationThread();
#endif

private:
    using OwnedArray = std::unique_ptr<Array>;

    struct Storage
    {
        int_t intCacheSize = 256;
        std::vector<OwnedArray> freeSmallArrays;
        std::vector<OwnedArray> inUseSmallArrays;
        std::vector<OwnedArray> freeLargeArrays;
        std::vector<OwnedArray> inUseLargeArrays;
        std::vector<OwnedArray> retiredLargeArrays;
    };

    // The only part that differs per platform: which Storage the calling thread
    // owns. Everything above operates on whatever this returns.
    static Storage &storage();

    static Array &acquire(std::vector<OwnedArray> &freeArrays,
                          std::vector<OwnedArray> &inUseArrays,
                          int_t size);
};
