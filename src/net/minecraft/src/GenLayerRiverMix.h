#pragma once

#include "GenLayer.h"

// net.minecraft.src.GenLayerRiverMix
class GenLayerRiverMix : public GenLayer
{
public:
    GenLayerRiverMix(long_t seed, Ptr biomeLayer, Ptr riverLayer);

    void initWorldGenSeed(long_t worldSeed) override;
    IntArray &getInts(int_t x, int_t z, int_t width, int_t height) override;

private:
    Ptr biomeLayer;
    Ptr riverLayer;
};
