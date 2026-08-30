#include "MapGenBase.h"

#include "World.h"
#include "java/Arithmetic.h"
#include "platform/PlatformTuning.h"

static_assert(PLATFORM_CAVE_SOURCE_RADIUS >= 0 && PLATFORM_CAVE_SOURCE_RADIUS <= 8,
              "Map generator source radius must stay within the vanilla sweep");

MapGenBase::MapGenBase()
    : sourceRange(PLATFORM_CAVE_SOURCE_RADIUS),
      range(8),
      worldObj(nullptr)
{
}

void MapGenBase::generate(IChunkProvider *, World *world,
                          int_t chunkX, int_t chunkZ, byte_t blocks[])
{
    worldObj = world;
    rand.setSeed(world->getRandomSeed());
    const long_t xMultiplier = rand.nextLong();
    const long_t zMultiplier = rand.nextLong();
    const long_t worldSeed = world->getRandomSeed();

    const int_t minSourceX = JavaArithmetic::intSub(chunkX, sourceRange);
    const int_t maxSourceX = JavaArithmetic::intAdd(chunkX, sourceRange);
    const int_t minSourceZ = JavaArithmetic::intSub(chunkZ, sourceRange);
    const int_t maxSourceZ = JavaArithmetic::intAdd(chunkZ, sourceRange);

    for (int_t sourceX = minSourceX; ; sourceX = JavaArithmetic::intAdd(sourceX, 1))
    {
        const long_t xSeed = JavaArithmetic::longMul(static_cast<long_t>(sourceX), xMultiplier);
        for (int_t sourceZ = minSourceZ; ; sourceZ = JavaArithmetic::intAdd(sourceZ, 1))
        {
            const long_t zSeed = JavaArithmetic::longMul(static_cast<long_t>(sourceZ), zMultiplier);
            const ulong_t seedBits = static_cast<ulong_t>(xSeed) ^
                                     static_cast<ulong_t>(zSeed) ^
                                     static_cast<ulong_t>(worldSeed);
            rand.setSeed(JavaArithmetic::longFromBits(seedBits));
            generateChunk(world, sourceX, sourceZ, chunkX, chunkZ, blocks);
            if (sourceZ == maxSourceZ)
                break;
        }
        if (sourceX == maxSourceX)
            break;
    }
}

void MapGenBase::generateChunk(World *, int_t, int_t, int_t, int_t, byte_t *)
{
}
