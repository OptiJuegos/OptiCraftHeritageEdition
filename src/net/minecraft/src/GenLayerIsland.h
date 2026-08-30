#pragma once

#include "GenLayer.h"

// net.minecraft.src.GenLayerIsland
class GenLayerIsland : public GenLayer
{
public:
    explicit GenLayerIsland(long_t seed);
    IntArray &getInts(int_t x, int_t z, int_t width, int_t height) override;
};
