#pragma once

#include <vector>

#include "GenLayer.h"

class BiomeGenBase;
class WorldType;

// net.minecraft.src.GenLayerBiome
class GenLayerBiome : public GenLayer
{
public:
    GenLayerBiome(long_t seed, Ptr parentLayer, WorldType *worldType);
    IntArray &getInts(int_t x, int_t z, int_t width, int_t height) override;

private:
    std::vector<BiomeGenBase *> allowedBiomes;
};
