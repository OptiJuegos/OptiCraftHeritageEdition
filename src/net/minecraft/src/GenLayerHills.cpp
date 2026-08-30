#include "GenLayerHills.h"
#include "java/Arithmetic.h"

#include "BiomeGenBase.h"
#include "IntCache.h"

GenLayerHills::GenLayerHills(long_t seed, Ptr parentLayer) : GenLayer(seed)
{
    parent = std::move(parentLayer);
}

GenLayer::IntArray &GenLayerHills::getInts(int_t x, int_t z, int_t width, int_t height)
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
            const int_t center = source[localX + 1 + (localZ + 1) * stride];
            if (nextInt(3) != 0)
            {
                result[localX + localZ * width] = center;
                continue;
            }

            int_t hill = center;
            if (center == BiomeGenBase::desert->biomeID) hill = BiomeGenBase::desertHills->biomeID;
            else if (center == BiomeGenBase::forest->biomeID) hill = BiomeGenBase::forestHills->biomeID;
            else if (center == BiomeGenBase::taiga->biomeID) hill = BiomeGenBase::taigaHills->biomeID;
            else if (center == BiomeGenBase::plains->biomeID) hill = BiomeGenBase::forest->biomeID;
            else if (center == BiomeGenBase::icePlains->biomeID) hill = BiomeGenBase::iceMountains->biomeID;
            else if (center == BiomeGenBase::jungle->biomeID) hill = BiomeGenBase::jungleHills->biomeID;

            if (hill != center)
            {
                const int_t north = source[localX + 1 + localZ * stride];
                const int_t east = source[localX + 2 + (localZ + 1) * stride];
                const int_t west = source[localX + (localZ + 1) * stride];
                const int_t south = source[localX + 1 + (localZ + 2) * stride];
                result[localX + localZ * width] =
                    north == center && east == center && west == center && south == center ? hill : center;
            }
            else
                result[localX + localZ * width] = center;
        }
    }

    return result;
}
