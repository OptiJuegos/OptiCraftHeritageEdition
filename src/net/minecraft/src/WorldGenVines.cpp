#include "WorldGenVines.h"
#include "java/Arithmetic.h"

#include "Block.h"
#include "BlockVine.h"
#include "World.h"

namespace
{
    constexpr int_t kFaceToSide[6] = {1, 0, 3, 2, 5, 4};
    constexpr int_t kVineGrowth[6] = {-1, -1, 2, 0, 1, 3};
}

bool WorldGenVines::generate(World *world, Random &random, int_t x, int_t y, int_t z)
{
    const int_t originX = x;
    const int_t originZ = z;

    for (; y < 128; ++y)
    {
        if (world->isAirBlock(x, y, z))
        {
            for (int_t side = 2; side <= 5; ++side)
            {
                if (Block::vine->canPlaceBlockOnSide(world, x, y, z, side))
                {
                    world->setBlockAndMetadata(x, y, z, Block::vine->blockID,
                                               1 << kVineGrowth[kFaceToSide[side]]);
                    break;
                }
            }
        }
        else
        {
            x = JavaArithmetic::intAdd(originX, random.nextIntDifference(4));
            z = JavaArithmetic::intAdd(originZ, random.nextIntDifference(4));
        }
    }

    return true;
}
