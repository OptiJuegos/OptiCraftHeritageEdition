#include "GenLayerAddSnow.h"
#include "java/Arithmetic.h"

#include "BiomeGenBase.h"
#include "IntCache.h"

GenLayerAddSnow::GenLayerAddSnow(long_t seed, Ptr parentLayer) : GenLayer(seed)
{
    parent = std::move(parentLayer);
}

GenLayer::IntArray &GenLayerAddSnow::getInts(int_t x, int_t z, int_t width, int_t height)
{
    const int_t parentX = JavaArithmetic::intSub(x, 1);
    const int_t parentZ = JavaArithmetic::intSub(z, 1);
    const int_t parentWidth = JavaArithmetic::intAdd(width, 2);
    const int_t parentHeight = JavaArithmetic::intAdd(height, 2);
    const IntArray &parentValues = parent->getInts(parentX, parentZ, parentWidth, parentHeight);
    IntArray &result = IntCache::getIntCache(IntCache::checkedAreaSize(width, height));

    for (int_t localZ = 0; localZ < height; ++localZ)
    {
        for (int_t localX = 0; localX < width; ++localX)
        {
            const int_t value = parentValues[localX + 1 + (localZ + 1) * parentWidth];
            initChunkSeed(static_cast<long_t>(JavaArithmetic::intAdd(localX, x)), static_cast<long_t>(JavaArithmetic::intAdd(localZ, z)));
            if (value == 0)
            {
                result[localX + localZ * width] = 0;
                continue;
            }

            int_t biome = nextInt(5);
            biome = biome == 0 ? BiomeGenBase::icePlains->biomeID : 1;
            result[localX + localZ * width] = biome;
        }
    }

    return result;
}
