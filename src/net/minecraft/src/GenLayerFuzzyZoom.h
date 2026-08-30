#pragma once

#include "GenLayerZoom.h"

// net.minecraft.src.GenLayerFuzzyZoom
class GenLayerFuzzyZoom : public GenLayerZoom
{
public:
    GenLayerFuzzyZoom(long_t seed, Ptr parentLayer);

protected:
    int_t choose(int_t first, int_t second, int_t third, int_t fourth) override;
};
