#include "GenLayerIsland.h"
#include "java/Arithmetic.h"

#include "IntCache.h"

GenLayerIsland::GenLayerIsland(long_t seed) : GenLayer(seed)
{
}

GenLayer::IntArray &GenLayerIsland::getInts(int_t x, int_t z, int_t width, int_t height)
{
    IntArray &result = IntCache::getIntCache(IntCache::checkedAreaSize(width, height));

    for (int_t localZ = 0; localZ < height; ++localZ)
    {
        for (int_t localX = 0; localX < width; ++localX)
        {
            initChunkSeed(static_cast<long_t>(JavaArithmetic::intAdd(x, localX)), static_cast<long_t>(JavaArithmetic::intAdd(z, localZ)));
            result[localX + localZ * width] = nextInt(10) == 0 ? 1 : 0;
        }
    }

    const int_t negWidth = JavaArithmetic::intSub(0, width);
    const int_t negHeight = JavaArithmetic::intSub(0, height);
    if (x > negWidth && x <= 0 && z > negHeight && z <= 0)
    {
        const int_t negX = JavaArithmetic::intSub(0, x);
        const int_t negZ = JavaArithmetic::intSub(0, z);
        result[JavaArithmetic::intAdd(negX, JavaArithmetic::intMul(negZ, width))] = 1;
    }

    return result;
}
