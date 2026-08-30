#include "WorldGenShrub.h"

#include "java/Arithmetic.h"
#include <cstdlib>

#include "Block.h"
#include "BlockGrass.h"
#include "BlockLeaves.h"
#include "World.h"

WorldGenShrub::WorldGenShrub(int_t woodMetadataValue, int_t leavesMetadataValue)
    : leavesMetadata(leavesMetadataValue),
      woodMetadata(woodMetadataValue)
{
}

bool WorldGenShrub::generate(World *world, Random &random, int_t x, int_t y, int_t z)
{
    while (true)
    {
        const int_t blockId = world->getBlockId(x, y, z);
        if ((blockId != 0 && blockId != Block::leaves->blockID) || y <= 0)
            break;
        y = JavaArithmetic::intSub(y, 1);
    }

    const int_t groundId = world->getBlockId(x, y, z);
    if (groundId != Block::dirt->blockID && groundId != Block::grass->blockID)
        return true;

    y = JavaArithmetic::intAdd(y, 1);
    setBlockAndMetadata(world, x, y, z, Block::wood->blockID, woodMetadata);

    for (int_t leafY = y; leafY <= JavaArithmetic::intAdd(y, 2); ++leafY)
    {
        const int_t relativeY = JavaArithmetic::intSub(leafY, y);
        const int_t radius = JavaArithmetic::intSub(2, relativeY);
        for (int_t leafX = JavaArithmetic::intSub(x, radius); leafX <= JavaArithmetic::intAdd(x, radius); ++leafX)
        {
            const int_t relativeX = JavaArithmetic::intSub(leafX, x);
            for (int_t leafZ = JavaArithmetic::intSub(z, radius); leafZ <= JavaArithmetic::intAdd(z, radius); ++leafZ)
            {
                const int_t relativeZ = JavaArithmetic::intSub(leafZ, z);
                if ((JavaArithmetic::intAbs(relativeX) == radius && JavaArithmetic::intAbs(relativeZ) == radius && random.nextInt(2) == 0) ||
                    Block::opaqueCubeLookup[world->getBlockId(leafX, leafY, leafZ)])
                    continue;

                setBlockAndMetadata(world, leafX, leafY, leafZ,
                                    Block::leaves->blockID, leavesMetadata);
            }
        }
    }

    return true;
}
