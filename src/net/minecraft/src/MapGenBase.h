#pragma once

#include "java/Random.h"
#include "java/Type.h"

class IChunkProvider;
class World;

// net.minecraft.src.MapGenBase
class MapGenBase
{
public:
    MapGenBase();
    virtual ~MapGenBase() = default;

    virtual void generate(IChunkProvider *provider, World *world,
                          int_t chunkX, int_t chunkZ, byte_t blocks[]);

protected:
    virtual void generateChunk(World *world, int_t sourceChunkX, int_t sourceChunkZ,
                               int_t targetChunkX, int_t targetChunkZ, byte_t blocks[]);

    int_t sourceRange;
    int_t range;
    Random rand;
    World *worldObj;
};
