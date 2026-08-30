#include "WorldGenTrees.h"

#include <cstdlib>

#include "Block.h"
#include "BlockGrass.h"
#include "BlockLeaves.h"
#include "BlockVine.h"
#include "World.h"
#include "java/Arithmetic.h"

WorldGenTrees::WorldGenTrees()
    : WorldGenTrees(false)
{
}

WorldGenTrees::WorldGenTrees(bool notify)
    : WorldGenTrees(notify, 4, 0, 0, false)
{
}

WorldGenTrees::WorldGenTrees(bool notify, int_t baseHeightValue, int_t woodMetadataValue,
                             int_t leavesMetadataValue, bool growVinesValue)
    : WorldGenerator(notify),
      baseHeight(baseHeightValue),
      growVines(growVinesValue),
      woodMetadata(woodMetadataValue),
      leavesMetadata(leavesMetadataValue)
{
}

bool WorldGenTrees::generate(World *world, Random &random, int_t x, int_t y, int_t z)
{
    const int_t height = JavaArithmetic::intAdd(random.nextInt(3), baseHeight);
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
                if (blockId != 0 && blockId != Block::leaves->blockID &&
                    blockId != Block::grass->blockID && blockId != Block::dirt->blockID &&
                    blockId != Block::wood->blockID)
                {
                    clear = false;
                }
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

    constexpr int_t leafDepth = 3;
    constexpr int_t initialRadius = 0;
    const int_t topY = JavaArithmetic::intAdd(y, height);
    for (int_t leafY = JavaArithmetic::intAdd(JavaArithmetic::intSub(y, leafDepth), height);
         leafY <= topY;
         leafY = JavaArithmetic::intAdd(leafY, 1))
    {
        const int_t relativeY = JavaArithmetic::intSub(leafY, topY);
        const int_t radius = initialRadius + 1 - relativeY / 2;
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
                                        Block::leaves->blockID, leavesMetadata);
                }
            }
        }
    }

    for (int_t trunkY = 0; trunkY < height; ++trunkY)
    {
        const int_t blockY = JavaArithmetic::intAdd(y, trunkY);
        const int_t blockId = world->getBlockId(x, blockY, z);
        if (blockId != 0 && blockId != Block::leaves->blockID)
            continue;

        setBlockAndMetadata(world, x, blockY, z, Block::wood->blockID, woodMetadata);
        if (!growVines || trunkY <= 0)
            continue;

        if (random.nextInt(3) > 0 && world->isAirBlock(JavaArithmetic::intSub(x, 1), blockY, z))
            setBlockAndMetadata(world, JavaArithmetic::intSub(x, 1), blockY, z, Block::vine->blockID, 8);
        if (random.nextInt(3) > 0 && world->isAirBlock(JavaArithmetic::intAdd(x, 1), blockY, z))
            setBlockAndMetadata(world, JavaArithmetic::intAdd(x, 1), blockY, z, Block::vine->blockID, 2);
        if (random.nextInt(3) > 0 && world->isAirBlock(x, blockY, JavaArithmetic::intSub(z, 1)))
            setBlockAndMetadata(world, x, blockY, JavaArithmetic::intSub(z, 1), Block::vine->blockID, 1);
        if (random.nextInt(3) > 0 && world->isAirBlock(x, blockY, JavaArithmetic::intAdd(z, 1)))
            setBlockAndMetadata(world, x, blockY, JavaArithmetic::intAdd(z, 1), Block::vine->blockID, 4);
    }

    if (!growVines)
        return true;

    for (int_t leafY = JavaArithmetic::intAdd(JavaArithmetic::intSub(y, 3), height);
         leafY <= topY;
         leafY = JavaArithmetic::intAdd(leafY, 1))
    {
        const int_t relativeY = JavaArithmetic::intSub(leafY, topY);
        const int_t radius = 2 - relativeY / 2;
        for (int_t leafX = JavaArithmetic::intSub(x, radius);
             leafX <= JavaArithmetic::intAdd(x, radius);
             leafX = JavaArithmetic::intAdd(leafX, 1))
        {
            for (int_t leafZ = JavaArithmetic::intSub(z, radius);
                 leafZ <= JavaArithmetic::intAdd(z, radius);
                 leafZ = JavaArithmetic::intAdd(leafZ, 1))
            {
                if (world->getBlockId(leafX, leafY, leafZ) != Block::leaves->blockID)
                    continue;
                if (random.nextInt(4) == 0 && world->getBlockId(JavaArithmetic::intSub(leafX, 1), leafY, leafZ) == 0)
                    growVinesDown(world, JavaArithmetic::intSub(leafX, 1), leafY, leafZ, 8);
                if (random.nextInt(4) == 0 && world->getBlockId(JavaArithmetic::intAdd(leafX, 1), leafY, leafZ) == 0)
                    growVinesDown(world, JavaArithmetic::intAdd(leafX, 1), leafY, leafZ, 2);
                if (random.nextInt(4) == 0 && world->getBlockId(leafX, leafY, JavaArithmetic::intSub(leafZ, 1)) == 0)
                    growVinesDown(world, leafX, leafY, JavaArithmetic::intSub(leafZ, 1), 1);
                if (random.nextInt(4) == 0 && world->getBlockId(leafX, leafY, JavaArithmetic::intAdd(leafZ, 1)) == 0)
                    growVinesDown(world, leafX, leafY, JavaArithmetic::intAdd(leafZ, 1), 4);
            }
        }
    }

    return true;
}

void WorldGenTrees::growVinesDown(World *world, int_t x, int_t y, int_t z, int_t metadata)
{
    setBlockAndMetadata(world, x, y, z, Block::vine->blockID, metadata);
    int_t remaining = 4;
    while (true)
    {
        y = JavaArithmetic::intSub(y, 1);
        if (world->getBlockId(x, y, z) != 0 || remaining <= 0)
            return;
        setBlockAndMetadata(world, x, y, z, Block::vine->blockID, metadata);
        remaining = JavaArithmetic::intSub(remaining, 1);
    }
}
