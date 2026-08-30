#pragma once

#include "GenLayer.h"

// net.minecraft.src.GenLayerRiverInit
class GenLayerRiverInit : public GenLayer
{
public:
    GenLayerRiverInit(long_t seed, Ptr parentLayer);
    IntArray &getInts(int_t x, int_t z, int_t width, int_t height) override;
};
