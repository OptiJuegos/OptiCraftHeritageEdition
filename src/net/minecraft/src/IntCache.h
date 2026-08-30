#pragma once

#include <memory>
#include <vector>

#include "java/Type.h"

// net.minecraft.src.IntCache
class IntCache
{
public:
    using Array = std::vector<int_t>;

    static Array &getIntCache(int_t requestedSize);
    static int_t checkedAreaSize(int_t width, int_t height);
    static void resetIntCache();

private:
    using OwnedArray = std::unique_ptr<Array>;

    static int_t intCacheSize;
    static std::vector<OwnedArray> freeSmallArrays;
    static std::vector<OwnedArray> inUseSmallArrays;
    static std::vector<OwnedArray> freeLargeArrays;
    static std::vector<OwnedArray> inUseLargeArrays;
    static std::vector<OwnedArray> retiredLargeArrays;

    static Array &acquire(std::vector<OwnedArray> &freeArrays,
                          std::vector<OwnedArray> &inUseArrays,
                          int_t size);
};
