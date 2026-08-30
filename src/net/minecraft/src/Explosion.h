#pragma once

#include <functional>
#include "java/HashSet.h"
#include "java/Random.h"
#include "java/Type.h"

class World;
class Entity;

struct ChunkPosTuple
{
    int_t x, y, z;
    bool operator==(const ChunkPosTuple &o) const
    {
        return x == o.x && y == o.y && z == o.z;
    }
};

struct ChunkPosTupleHash
{
    std::size_t operator()(const ChunkPosTuple &p) const;
};

// net.minecraft.src.Explosion
class Explosion
{
public:
    Explosion(World *world, Entity *entity, double x, double y, double z, float size);

    void doExplosionA();
    void doExplosionB(bool flag);

    bool isFlaming;
    double explosionX;
    double explosionY;
    double explosionZ;
    Entity *exploder;
    float explosionSize;
    JavaHashSet<ChunkPosTuple, ChunkPosTupleHash, std::equal_to<ChunkPosTuple>> destroyedBlockPositions;

private:
    Random explosionRNG;
    World *worldObj;
};
