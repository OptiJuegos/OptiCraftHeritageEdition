#include "WorldGenSpikes.h"

#include "Block.h"
#include "EntityEnderCrystal.h"
#include "World.h"
#include "java/Arithmetic.h"

WorldGenSpikes::WorldGenSpikes(int_t replaceId)
    : replaceID(replaceId)
{
}

bool WorldGenSpikes::generate(World *world, Random &random, int_t x, int_t y, int_t z)
{
    if (!world->isAirBlock(x, y, z) || world->getBlockId(x, JavaArithmetic::intSub(y, 1), z) != replaceID)
        return false;

    const int_t height = JavaArithmetic::intAdd(random.nextInt(32), 6);
    const int_t radius = JavaArithmetic::intAdd(random.nextInt(4), 1);
    const int_t minX = JavaArithmetic::intSub(x, radius);
    const int_t maxX = JavaArithmetic::intAdd(x, radius);
    const int_t minZ = JavaArithmetic::intSub(z, radius);
    const int_t maxZ = JavaArithmetic::intAdd(z, radius);
    const int_t topY = JavaArithmetic::intAdd(y, height);
    const int_t maxRadiusSq = JavaArithmetic::intAdd(JavaArithmetic::intMul(radius, radius), 1);

    for (int_t checkX = minX; checkX <= maxX; checkX = JavaArithmetic::intAdd(checkX, 1))
    {
        for (int_t checkZ = minZ; checkZ <= maxZ; checkZ = JavaArithmetic::intAdd(checkZ, 1))
        {
            const int_t dx = JavaArithmetic::intSub(checkX, x);
            const int_t dz = JavaArithmetic::intSub(checkZ, z);
            const int_t distanceSq = JavaArithmetic::intAdd(JavaArithmetic::intMul(dx, dx), JavaArithmetic::intMul(dz, dz));
            if (distanceSq <= maxRadiusSq &&
                world->getBlockId(checkX, JavaArithmetic::intSub(y, 1), checkZ) != replaceID)
            {
                return false;
            }
        }
    }

    for (int_t blockY = y; blockY < topY && blockY < 128; blockY = JavaArithmetic::intAdd(blockY, 1))
    {
        for (int_t blockX = minX; blockX <= maxX; blockX = JavaArithmetic::intAdd(blockX, 1))
        {
            for (int_t blockZ = minZ; blockZ <= maxZ; blockZ = JavaArithmetic::intAdd(blockZ, 1))
            {
                const int_t dx = JavaArithmetic::intSub(blockX, x);
                const int_t dz = JavaArithmetic::intSub(blockZ, z);
                const int_t distanceSq = JavaArithmetic::intAdd(JavaArithmetic::intMul(dx, dx), JavaArithmetic::intMul(dz, dz));
                if (distanceSq <= maxRadiusSq)
                    world->setBlockWithNotify(blockX, blockY, blockZ, Block::obsidian->blockID);
            }
        }
    }

    EntityEnderCrystal *crystal = new EntityEnderCrystal(world);
    crystal->setLocationAndAngles(static_cast<double>(static_cast<float>(x) + 0.5f),
                                  static_cast<double>(topY),
                                  static_cast<double>(static_cast<float>(z) + 0.5f),
                                  random.nextFloat() * 360.0f, 0.0f);
    if (!world->spawnEntityInWorld(crystal))
        delete crystal;
    world->setBlockWithNotify(x, topY, z, Block::bedrock->blockID);
    return true;
}
