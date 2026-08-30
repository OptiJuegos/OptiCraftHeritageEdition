#pragma once

#include "GenLayer.h"

// net.minecraft.src.GenLayerZoom
class GenLayerZoom : public GenLayer
{
public:
    GenLayerZoom(long_t seed, Ptr parentLayer);

    IntArray &getInts(int_t x, int_t z, int_t width, int_t height) override;
    static Ptr func_35515_a(long_t seed, Ptr layer, int_t count);

protected:
    virtual int_t choose(int_t first, int_t second);
    virtual int_t choose(int_t first, int_t second, int_t third, int_t fourth);
    int_t func_35514_b(int_t first, int_t second, int_t third, int_t fourth);
};
