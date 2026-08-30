#include "GenLayerRiverInit.h"
#include "java/Arithmetic.h"

#include "IntCache.h"

GenLayerRiverInit::GenLayerRiverInit(long_t seed, Ptr parentLayer) : GenLayer(seed)
{
    parent = std::move(parentLayer);
}

GenLayer::IntArray &GenLayerRiverInit::getInts(int_t x, int_t z, int_t width, int_t height)
{
    const IntArray &source = parent->getInts(x, z, width, height);
    IntArray &result = IntCache::getIntCache(IntCache::checkedAreaSize(width, height));

    for (int_t localZ = 0; localZ < height; ++localZ)
    {
        for (int_t localX = 0; localX < width; ++localX)
        {
            initChunkSeed(static_cast<long_t>(JavaArithmetic::intAdd(localX, x)), static_cast<long_t>(JavaArithmetic::intAdd(localZ, z)));
            const int_t index = localX + localZ * width;
            result[index] = source[index] > 0 ? nextInt(2) + 2 : 0;
        }
    }

    return result;
}
