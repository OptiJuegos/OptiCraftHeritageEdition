#include "GenLayerAddMushroomIsland.h"
#include "java/Arithmetic.h"

#include "BiomeGenBase.h"
#include "IntCache.h"

GenLayerAddMushroomIsland::GenLayerAddMushroomIsland(long_t seed, Ptr parentLayer) : GenLayer(seed)
{
    parent = std::move(parentLayer);
}

GenLayer::IntArray &GenLayerAddMushroomIsland::getInts(int_t x, int_t z, int_t width, int_t height)
{
    const int_t parentX = JavaArithmetic::intSub(x, 1);
    const int_t parentZ = JavaArithmetic::intSub(z, 1);
    const int_t parentWidth = JavaArithmetic::intAdd(width, 2);
    const int_t parentHeight = JavaArithmetic::intAdd(height, 2);
    const IntArray &source = parent->getInts(parentX, parentZ, parentWidth, parentHeight);
    IntArray &result = IntCache::getIntCache(IntCache::checkedAreaSize(width, height));

    for (int_t localZ = 0; localZ < height; ++localZ)
    {
        for (int_t localX = 0; localX < width; ++localX)
        {
            const int_t nw = source[localX + (localZ)*parentWidth];
            const int_t ne = source[localX + 2 + (localZ)*parentWidth];
            const int_t sw = source[localX + (localZ + 2) * parentWidth];
            const int_t se = source[localX + 2 + (localZ + 2) * parentWidth];
            const int_t center = source[localX + 1 + (localZ + 1) * parentWidth];
            initChunkSeed(static_cast<long_t>(JavaArithmetic::intAdd(localX, x)), static_cast<long_t>(JavaArithmetic::intAdd(localZ, z)));

            result[localX + localZ * width] =
                center == 0 && nw == 0 && ne == 0 && sw == 0 && se == 0 && nextInt(100) == 0
                    ? BiomeGenBase::mushroomIsland->biomeID
                    : center;
        }
    }

    return result;
}
