#include "GenLayerBiome.h"
#include "java/Arithmetic.h"

#include "BiomeGenBase.h"
#include "IntCache.h"
#include "WorldType.h"

GenLayerBiome::GenLayerBiome(long_t seed, Ptr parentLayer, WorldType *worldType)
    : GenLayer(seed)
{
    parent = std::move(parentLayer);
    allowedBiomes = {
        BiomeGenBase::desert,
        BiomeGenBase::forest,
        BiomeGenBase::extremeHills,
        BiomeGenBase::swampland,
        BiomeGenBase::plains,
        BiomeGenBase::taiga,
        BiomeGenBase::jungle
    };

    if (worldType == WorldType::DEFAULT_1_1)
        allowedBiomes.pop_back();
}

GenLayer::IntArray &GenLayerBiome::getInts(int_t x, int_t z, int_t width, int_t height)
{
    const IntArray &source = parent->getInts(x, z, width, height);
    IntArray &result = IntCache::getIntCache(IntCache::checkedAreaSize(width, height));

    for (int_t localZ = 0; localZ < height; ++localZ)
    {
        for (int_t localX = 0; localX < width; ++localX)
        {
            initChunkSeed(static_cast<long_t>(JavaArithmetic::intAdd(localX, x)), static_cast<long_t>(JavaArithmetic::intAdd(localZ, z)));
            const int_t value = source[localX + localZ * width];
            if (value == 0)
                result[localX + localZ * width] = 0;
            else if (value == BiomeGenBase::mushroomIsland->biomeID)
                result[localX + localZ * width] = value;
            else if (value == 1)
                result[localX + localZ * width] = allowedBiomes[static_cast<std::size_t>(nextInt(static_cast<int_t>(allowedBiomes.size())))]->biomeID;
            else
                result[localX + localZ * width] = BiomeGenBase::icePlains->biomeID;
        }
    }

    return result;
}
