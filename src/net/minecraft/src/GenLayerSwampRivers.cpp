#include "GenLayerSwampRivers.h"
#include "java/Arithmetic.h"

#include "BiomeGenBase.h"
#include "IntCache.h"

GenLayerSwampRivers::GenLayerSwampRivers(long_t seed, Ptr parentLayer) : GenLayer(seed)
{
    parent = std::move(parentLayer);
}

GenLayer::IntArray &GenLayerSwampRivers::getInts(int_t x, int_t z, int_t width, int_t height)
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
            initChunkSeed(static_cast<long_t>(JavaArithmetic::intAdd(localX, x)), static_cast<long_t>(JavaArithmetic::intAdd(localZ, z)));
            const int_t biome = source[localX + 1 + (localZ + 1) * stride];
            if (biome == BiomeGenBase::swampland->biomeID && nextInt(6) == 0)
                result[localX + localZ * width] = BiomeGenBase::river->biomeID;
            else if ((biome == BiomeGenBase::jungle->biomeID || biome == BiomeGenBase::jungleHills->biomeID) && nextInt(8) == 0)
                result[localX + localZ * width] = BiomeGenBase::river->biomeID;
            else
                result[localX + localZ * width] = biome;
        }
    }

    return result;
}
