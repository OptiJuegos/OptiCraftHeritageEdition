#include "WorldGenWaterlily.h"
#include "java/Arithmetic.h"

#include "Block.h"
#include "BlockLilyPad.h"
#include "World.h"

bool WorldGenWaterlily::generate(World *world, Random &random, int_t x, int_t y, int_t z)
{
    for (int_t i = 0; i < 10; ++i)
    {
        const int_t blockX = random.nextIntOffset(x, 8);
        const int_t blockY = random.nextIntOffset(y, 4);
        const int_t blockZ = random.nextIntOffset(z, 8);
        if (world->isAirBlock(blockX, blockY, blockZ) &&
            Block::waterlily->canPlaceBlockAt(world, blockX, blockY, blockZ))
        {
            world->setBlock(blockX, blockY, blockZ, Block::waterlily->blockID);
        }
    }
    return true;
}
