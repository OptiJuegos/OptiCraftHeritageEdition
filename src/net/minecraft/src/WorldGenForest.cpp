#include "WorldGenForest.h"

#include <cstdlib>

#include "Block.h"
#include "BlockGrass.h"
#include "BlockLeaves.h"
#include "World.h"
#include "java/Arithmetic.h"

WorldGenForest::WorldGenForest()
    : WorldGenForest(false)
{
}

WorldGenForest::WorldGenForest(bool notify)
    : WorldGenerator(notify)
{
}

bool WorldGenForest::generate(World *world, Random &random, int_t x, int_t y, int_t z)
{
    const int_t height = JavaArithmetic::intAdd(random.nextInt(3), 5);
    bool clear = true;
    if (y < 1 || JavaArithmetic::intAdd(JavaArithmetic::intAdd(y, height), 1) > 256)
        return false;

    for (int_t checkY = y;
         checkY <= JavaArithmetic::intAdd(JavaArithmetic::intAdd(y, 1), height) && clear;
         checkY = JavaArithmetic::intAdd(checkY, 1))
    {
        int_t radius = 1;
        if (checkY == y)
            radius = 0;
        if (checkY >= JavaArithmetic::intSub(
                          JavaArithmetic::intAdd(JavaArithmetic::intAdd(y, 1), height), 2))
            radius = 2;

        for (int_t checkX = JavaArithmetic::intSub(x, radius);
             checkX <= JavaArithmetic::intAdd(x, radius) && clear;
             checkX = JavaArithmetic::intAdd(checkX, 1))
        {
            for (int_t checkZ = JavaArithmetic::intSub(z, radius);
                 checkZ <= JavaArithmetic::intAdd(z, radius) && clear;
                 checkZ = JavaArithmetic::intAdd(checkZ, 1))
            {
                if (checkY < 0 || checkY >= 256)
                {
                    clear = false;
                    break;
                }
                const int_t blockId = world->getBlockId(checkX, checkY, checkZ);
                if (blockId != 0 && blockId != Block::leaves->blockID)
                    clear = false;
            }
        }
    }

    if (!clear)
        return false;

    const int_t groundY = JavaArithmetic::intSub(y, 1);
    const int_t groundId = world->getBlockId(x, groundY, z);
    if ((groundId != Block::grass->blockID && groundId != Block::dirt->blockID) ||
        y >= JavaArithmetic::intSub(JavaArithmetic::intSub(256, height), 1))
        return false;

    setBlock(world, x, groundY, z, Block::dirt->blockID);

    const int_t topY = JavaArithmetic::intAdd(y, height);
    for (int_t leafY = JavaArithmetic::intAdd(JavaArithmetic::intSub(y, 3), height);
         leafY <= topY;
         leafY = JavaArithmetic::intAdd(leafY, 1))
    {
        const int_t relativeY = JavaArithmetic::intSub(leafY, topY);
        const int_t radius = 1 - relativeY / 2;
        for (int_t leafX = JavaArithmetic::intSub(x, radius);
             leafX <= JavaArithmetic::intAdd(x, radius);
             leafX = JavaArithmetic::intAdd(leafX, 1))
        {
            const int_t relativeX = JavaArithmetic::intSub(leafX, x);
            for (int_t leafZ = JavaArithmetic::intSub(z, radius);
                 leafZ <= JavaArithmetic::intAdd(z, radius);
                 leafZ = JavaArithmetic::intAdd(leafZ, 1))
            {
                const int_t relativeZ = JavaArithmetic::intSub(leafZ, z);
                if ((JavaArithmetic::intAbs(relativeX) != radius ||
                     JavaArithmetic::intAbs(relativeZ) != radius ||
                     random.nextInt(2) != 0 && relativeY != 0) &&
                    !Block::opaqueCubeLookup[world->getBlockId(leafX, leafY, leafZ)])
                {
                    setBlockAndMetadata(world, leafX, leafY, leafZ,
                                        Block::leaves->blockID, 2);
                }
            }
        }
    }

    for (int_t trunkY = 0; trunkY < height; ++trunkY)
    {
        const int_t blockY = JavaArithmetic::intAdd(y, trunkY);
        const int_t blockId = world->getBlockId(x, blockY, z);
        if (blockId == 0 || blockId == Block::leaves->blockID)
            setBlockAndMetadata(world, x, blockY, z, Block::wood->blockID, 2);
    }

    return true;
}
