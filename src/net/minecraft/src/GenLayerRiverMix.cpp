#include "GenLayerRiverMix.h"
#include "java/Arithmetic.h"

#include "BiomeGenBase.h"
#include "IntCache.h"

GenLayerRiverMix::GenLayerRiverMix(long_t seed, Ptr biomeLayerValue, Ptr riverLayerValue)
    : GenLayer(seed),
      biomeLayer(std::move(biomeLayerValue)),
      riverLayer(std::move(riverLayerValue))
{
}

void GenLayerRiverMix::initWorldGenSeed(long_t worldSeed)
{
    biomeLayer->initWorldGenSeed(worldSeed);
    riverLayer->initWorldGenSeed(worldSeed);
    GenLayer::initWorldGenSeed(worldSeed);
}

GenLayer::IntArray &GenLayerRiverMix::getInts(int_t x, int_t z, int_t width, int_t height)
{
    const IntArray &biomes = biomeLayer->getInts(x, z, width, height);
    const IntArray &rivers = riverLayer->getInts(x, z, width, height);
    const int_t count = IntCache::checkedAreaSize(width, height);
    IntArray &result = IntCache::getIntCache(count);

    for (int_t index = 0; index < count; ++index)
    {
        if (biomes[index] == BiomeGenBase::ocean->biomeID)
            result[index] = biomes[index];
        else if (rivers[index] >= 0)
        {
            if (biomes[index] == BiomeGenBase::icePlains->biomeID)
                result[index] = BiomeGenBase::frozenRiver->biomeID;
            else if (biomes[index] != BiomeGenBase::mushroomIsland->biomeID &&
                     biomes[index] != BiomeGenBase::mushroomIslandShore->biomeID)
                result[index] = rivers[index];
            else
                result[index] = BiomeGenBase::mushroomIslandShore->biomeID;
        }
        else
            result[index] = biomes[index];
    }

    return result;
}
