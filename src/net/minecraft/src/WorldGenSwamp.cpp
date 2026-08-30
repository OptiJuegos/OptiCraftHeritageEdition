#include "WorldGenSwamp.h"

#include <cstdlib>

#include "Block.h"
#include "BlockGrass.h"
#include "BlockLeaves.h"
#include "BlockVine.h"
#include "Material.h"
#include "World.h"
#include "java/Arithmetic.h"

bool WorldGenSwamp::generate(World *world, Random &random, int_t x, int_t y, int_t z)
{
    const int_t height = JavaArithmetic::intAdd(random.nextInt(4), 5);
    while (world->getBlockMaterial(x, JavaArithmetic::intSub(y, 1), z) == Material::water)
        y = JavaArithmetic::intSub(y, 1);

    bool clear = true;
    if (y < 1 || JavaArithmetic::intAdd(JavaArithmetic::intAdd(y, height), 1) > 128)
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
            radius = 3;

        for (int_t checkX = JavaArithmetic::intSub(x, radius);
             checkX <= JavaArithmetic::intAdd(x, radius) && clear;
             checkX = JavaArithmetic::intAdd(checkX, 1))
        {
            for (int_t checkZ = JavaArithmetic::intSub(z, radius);
                 checkZ <= JavaArithmetic::intAdd(z, radius) && clear;
                 checkZ = JavaArithmetic::intAdd(checkZ, 1))
            {
                if (checkY < 0 || checkY >= 128)
                {
                    clear = false;
                    break;
                }

                const int_t blockId = world->getBlockId(checkX, checkY, checkZ);
                if (blockId == 0 || blockId == Block::leaves->blockID)
                    continue;
                if (blockId != Block::waterStill->blockID && blockId != Block::waterMoving->blockID)
                    clear = false;
                else if (checkY > y)
                    clear = false;
            }
        }
    }

    if (!clear)
        return false;

    const int_t groundY = JavaArithmetic::intSub(y, 1);
    const int_t groundId = world->getBlockId(x, groundY, z);
    if ((groundId != Block::grass->blockID && groundId != Block::dirt->blockID) ||
        y >= JavaArithmetic::intSub(JavaArithmetic::intSub(128, height), 1))
        return false;

    setBlock(world, x, groundY, z, Block::dirt->blockID);

    const int_t topY = JavaArithmetic::intAdd(y, height);
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
                    setBlock(world, leafX, leafY, leafZ, Block::leaves->blockID);
                }
            }
        }
    }

    for (int_t trunkY = 0; trunkY < height; ++trunkY)
    {
        const int_t blockY = JavaArithmetic::intAdd(y, trunkY);
        const int_t blockId = world->getBlockId(x, blockY, z);
        if (blockId == 0 || blockId == Block::leaves->blockID ||
            blockId == Block::waterMoving->blockID || blockId == Block::waterStill->blockID)
        {
            setBlock(world, x, blockY, z, Block::wood->blockID);
        }
    }

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
                    generateVines(world, JavaArithmetic::intSub(leafX, 1), leafY, leafZ, 8);
                if (random.nextInt(4) == 0 && world->getBlockId(JavaArithmetic::intAdd(leafX, 1), leafY, leafZ) == 0)
                    generateVines(world, JavaArithmetic::intAdd(leafX, 1), leafY, leafZ, 2);
                if (random.nextInt(4) == 0 && world->getBlockId(leafX, leafY, JavaArithmetic::intSub(leafZ, 1)) == 0)
                    generateVines(world, leafX, leafY, JavaArithmetic::intSub(leafZ, 1), 1);
                if (random.nextInt(4) == 0 && world->getBlockId(leafX, leafY, JavaArithmetic::intAdd(leafZ, 1)) == 0)
                    generateVines(world, leafX, leafY, JavaArithmetic::intAdd(leafZ, 1), 4);
            }
        }
    }

    return true;
}

void WorldGenSwamp::generateVines(World *world, int_t x, int_t y, int_t z, int_t metadata)
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
