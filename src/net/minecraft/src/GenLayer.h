#pragma once

#include <array>
#include <memory>
#include <vector>

#include "java/Type.h"

class WorldType;

// net.minecraft.src.GenLayer
class GenLayer
{
public:
    using Ptr = std::shared_ptr<GenLayer>;
    using IntArray = std::vector<int_t>;

    explicit GenLayer(long_t seed);
    virtual ~GenLayer() = default;

    static std::array<Ptr, 3> func_48425_a(long_t worldSeed, WorldType *worldType);

    virtual void initWorldGenSeed(long_t worldSeed);
    void initChunkSeed(long_t x, long_t z);
    virtual IntArray &getInts(int_t x, int_t z, int_t width, int_t height) = 0;

protected:
    int_t nextInt(int_t bound);

    Ptr parent;

private:
    static long_t mixSeed(long_t seed, long_t addend);

    long_t worldGenSeed;
    long_t chunkSeed;
    long_t baseSeed;
};
