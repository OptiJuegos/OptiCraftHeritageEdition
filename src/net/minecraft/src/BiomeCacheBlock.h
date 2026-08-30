#pragma once

#include <vector>

#include "NoiseBuffer.h"
#include "java/Type.h"

class BiomeCache;
class BiomeGenBase;

// net.minecraft.src.BiomeCacheBlock
class BiomeCacheBlock
{
public:
    BiomeCacheBlock(BiomeCache *cache, int_t chunkX, int_t chunkZ);

    BiomeGenBase *getBiomeGenAt(int_t x, int_t z) const;

    BiomeNoiseBuffer temperatureValues;
    BiomeNoiseBuffer rainfallValues;
    std::vector<BiomeGenBase *> biomes;
    int_t xPosition;
    int_t zPosition;
    long_t lastAccessTime;

private:
    BiomeCache *biomeCache;
};
