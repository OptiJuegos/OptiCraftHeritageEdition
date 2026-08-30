#pragma once

#include "java/Random.h"
#include "java/Type.h"

class World;

// net.minecraft.src.WorldGenerator
class WorldGenerator
{
public:
    WorldGenerator();
    explicit WorldGenerator(bool doBlockNotify);
    virtual ~WorldGenerator() = default;

    virtual bool generate(World *world, Random &random, int_t i, int_t j, int_t k) = 0;

    virtual void setScale(double d, double d1, double d2) {}
    virtual void getEntityString(double d, double d1, double d2) { setScale(d, d1, d2); }

protected:
    void setBlock(World *world, int_t x, int_t y, int_t z, int_t blockId);
    void setBlockAndMetadata(World *world, int_t x, int_t y, int_t z,
                             int_t blockId, int_t metadata);

private:
    bool doBlockNotify;
};
