#include "WorldGenDesertWells.h"

#include "java/Arithmetic.h"
#include "Block.h"
#include "World.h"

bool WorldGenDesertWells::generate(World *world, Random &random, int_t x, int_t y, int_t z)
{
    while (world->isAirBlock(x, y, z) && y > 2)
        y = JavaArithmetic::intSub(y, 1);

    if (world->getBlockId(x, y, z) != Block::sand->blockID)
        return false;

    for (int_t dx = -2; dx <= 2; ++dx)
    {
        for (int_t dz = -2; dz <= 2; ++dz)
        {
            if (world->isAirBlock(JavaArithmetic::intAdd(x, dx), JavaArithmetic::intSub(y, 1), JavaArithmetic::intAdd(z, dz)) &&
                world->isAirBlock(JavaArithmetic::intAdd(x, dx), JavaArithmetic::intSub(y, 2), JavaArithmetic::intAdd(z, dz)))
                return false;
        }
    }

    for (int_t dy = -1; dy <= 0; ++dy)
    {
        for (int_t dx = -2; dx <= 2; ++dx)
        {
            for (int_t dz = -2; dz <= 2; ++dz)
                world->setBlock(JavaArithmetic::intAdd(x, dx), JavaArithmetic::intAdd(y, dy), JavaArithmetic::intAdd(z, dz), Block::sandStone->blockID);
        }
    }

    world->setBlock(x, y, z, Block::waterMoving->blockID);
    world->setBlock(JavaArithmetic::intSub(x, 1), y, z, Block::waterMoving->blockID);
    world->setBlock(JavaArithmetic::intAdd(x, 1), y, z, Block::waterMoving->blockID);
    world->setBlock(x, y, JavaArithmetic::intSub(z, 1), Block::waterMoving->blockID);
    world->setBlock(x, y, JavaArithmetic::intAdd(z, 1), Block::waterMoving->blockID);

    for (int_t dx = -2; dx <= 2; ++dx)
    {
        for (int_t dz = -2; dz <= 2; ++dz)
        {
            if (dx == -2 || dx == 2 || dz == -2 || dz == 2)
                world->setBlock(JavaArithmetic::intAdd(x, dx), JavaArithmetic::intAdd(y, 1), JavaArithmetic::intAdd(z, dz), Block::sandStone->blockID);
        }
    }

    world->setBlockAndMetadata(JavaArithmetic::intAdd(x, 2), JavaArithmetic::intAdd(y, 1), z, Block::stairSingle->blockID, 1);
    world->setBlockAndMetadata(JavaArithmetic::intSub(x, 2), JavaArithmetic::intAdd(y, 1), z, Block::stairSingle->blockID, 1);
    world->setBlockAndMetadata(x, JavaArithmetic::intAdd(y, 1), JavaArithmetic::intAdd(z, 2), Block::stairSingle->blockID, 1);
    world->setBlockAndMetadata(x, JavaArithmetic::intAdd(y, 1), JavaArithmetic::intSub(z, 2), Block::stairSingle->blockID, 1);

    for (int_t dx = -1; dx <= 1; ++dx)
    {
        for (int_t dz = -1; dz <= 1; ++dz)
        {
            if (dx == 0 && dz == 0)
                world->setBlock(JavaArithmetic::intAdd(x, dx), JavaArithmetic::intAdd(y, 4), JavaArithmetic::intAdd(z, dz), Block::sandStone->blockID);
            else
                world->setBlockAndMetadata(JavaArithmetic::intAdd(x, dx), JavaArithmetic::intAdd(y, 4), JavaArithmetic::intAdd(z, dz), Block::stairSingle->blockID, 1);
        }
    }

    for (int_t dy = 1; dy <= 3; ++dy)
    {
        world->setBlock(JavaArithmetic::intSub(x, 1), JavaArithmetic::intAdd(y, dy), JavaArithmetic::intSub(z, 1), Block::sandStone->blockID);
        world->setBlock(JavaArithmetic::intSub(x, 1), JavaArithmetic::intAdd(y, dy), JavaArithmetic::intAdd(z, 1), Block::sandStone->blockID);
        world->setBlock(JavaArithmetic::intAdd(x, 1), JavaArithmetic::intAdd(y, dy), JavaArithmetic::intSub(z, 1), Block::sandStone->blockID);
        world->setBlock(JavaArithmetic::intAdd(x, 1), JavaArithmetic::intAdd(y, dy), JavaArithmetic::intAdd(z, 1), Block::sandStone->blockID);
    }

    return true;
}
