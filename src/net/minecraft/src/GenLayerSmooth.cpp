#include "GenLayerSmooth.h"
#include "java/Arithmetic.h"

#include "IntCache.h"

GenLayerSmooth::GenLayerSmooth(long_t seed, Ptr parentLayer) : GenLayer(seed)
{
    parent = std::move(parentLayer);
}

GenLayer::IntArray &GenLayerSmooth::getInts(int_t x, int_t z, int_t width, int_t height)
{
    const int_t parentX = JavaArithmetic::intSub(x, 1);
    const int_t parentZ = JavaArithmetic::intSub(z, 1);
    const int_t stride = JavaArithmetic::intAdd(width, 2);
    const int_t parentHeight = JavaArithmetic::intAdd(height, 2);
    const IntArray &source = parent->getInts(parentX, parentZ, stride, parentHeight);
    IntArray &result = IntCache::getIntCache(IntCache::checkedAreaSize(width, height));

    for (int_t localZ = 0; localZ < height; ++localZ)
    {
        for (int_t localX = 0; localX < width; ++localX)
        {
            const int_t west = source[localX + (localZ + 1) * stride];
            const int_t east = source[localX + 2 + (localZ + 1) * stride];
            const int_t north = source[localX + 1 + localZ * stride];
            const int_t south = source[localX + 1 + (localZ + 2) * stride];
            int_t center = source[localX + 1 + (localZ + 1) * stride];

            if (west == east && north == south)
            {
                initChunkSeed(static_cast<long_t>(JavaArithmetic::intAdd(localX, x)), static_cast<long_t>(JavaArithmetic::intAdd(localZ, z)));
                center = nextInt(2) == 0 ? west : north;
            }
            else
            {
                if (west == east)
                    center = west;
                if (north == south)
                    center = north;
            }

            result[localX + localZ * width] = center;
        }
    }

    return result;
}
