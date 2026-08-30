#include "WorldGenHugeTrees.h"

#include "Block.h"
#include "BlockGrass.h"
#include "BlockLeaves.h"
#include "BlockVine.h"
#include "MathHelper.h"
#include "World.h"
#include "java/Arithmetic.h"

WorldGenHugeTrees::WorldGenHugeTrees(bool notify, int_t baseHeightValue,
                                     int_t woodMetadataValue, int_t leavesMetadataValue)
    : WorldGenerator(notify),
      baseHeight(baseHeightValue),
      woodMetadata(woodMetadataValue),
      leavesMetadata(leavesMetadataValue)
{
}

bool WorldGenHugeTrees::generate(World *world, Random &random, int_t x, int_t y, int_t z)
{
    const int_t height = JavaArithmetic::intAdd(random.nextInt(3), baseHeight);
    bool clear = true;
    if (y < 1 || JavaArithmetic::intAdd(JavaArithmetic::intAdd(y, height), 1) > 256)
        return false;

    for (int_t checkY = y; checkY <= JavaArithmetic::intAdd(JavaArithmetic::intAdd(y, 1), height) && clear; ++checkY)
    {
        int_t radius = 2;
        if (checkY == y)
            radius = 1;
        if (checkY >= JavaArithmetic::intSub(JavaArithmetic::intAdd(JavaArithmetic::intAdd(y, 1), height), 2))
            radius = 2;

        for (int_t checkX = JavaArithmetic::intSub(x, radius); checkX <= JavaArithmetic::intAdd(x, radius) && clear; ++checkX)
        {
            for (int_t checkZ = JavaArithmetic::intSub(z, radius); checkZ <= JavaArithmetic::intAdd(z, radius) && clear; ++checkZ)
            {
                if (checkY < 0 || checkY >= 256)
                {
                    clear = false;
                    break;
                }

                const int_t blockId = world->getBlockId(checkX, checkY, checkZ);
                if (blockId != 0 && blockId != Block::leaves->blockID &&
                    blockId != Block::grass->blockID && blockId != Block::dirt->blockID &&
                    blockId != Block::wood->blockID && blockId != Block::sapling->blockID)
                {
                    clear = false;
                }
            }
        }
    }

    if (!clear)
        return false;

    const int_t groundId = world->getBlockId(x, JavaArithmetic::intSub(y, 1), z);
    if ((groundId != Block::grass->blockID && groundId != Block::dirt->blockID) ||
        y >= JavaArithmetic::intSub(JavaArithmetic::intSub(256, height), 1))
        return false;

    world->setBlock(x, JavaArithmetic::intSub(y, 1), z, Block::dirt->blockID);
    world->setBlock(JavaArithmetic::intAdd(x, 1), JavaArithmetic::intSub(y, 1), z, Block::dirt->blockID);
    world->setBlock(x, JavaArithmetic::intSub(y, 1), JavaArithmetic::intAdd(z, 1), Block::dirt->blockID);
    world->setBlock(JavaArithmetic::intAdd(x, 1), JavaArithmetic::intSub(y, 1), JavaArithmetic::intAdd(z, 1), Block::dirt->blockID);
    generateLeafNode(world, x, z, JavaArithmetic::intAdd(y, height), 2, random);

    for (int_t branchY = JavaArithmetic::intSub(JavaArithmetic::intSub(JavaArithmetic::intAdd(y, height), 2), random.nextInt(4));
         branchY > JavaArithmetic::intAdd(y, height / 2);
         branchY = JavaArithmetic::intSub(branchY, JavaArithmetic::intAdd(2, random.nextInt(4))))
    {
        const float angle = random.nextFloat() * 3.1415927f * 2.0f;
        int_t branchX = JavaArithmetic::intAdd(x, JavaArithmetic::floatToInt(0.5f + MathHelper::cos(angle) * 4.0f));
        int_t branchZ = JavaArithmetic::intAdd(z, JavaArithmetic::floatToInt(0.5f + MathHelper::sin(angle) * 4.0f));
        generateLeafNode(world, branchX, branchZ, branchY, 0, random);

        for (int_t step = 0; step < 5; ++step)
        {
            branchX = JavaArithmetic::intAdd(x, JavaArithmetic::floatToInt(1.5f + MathHelper::cos(angle) * (float)step));
            branchZ = JavaArithmetic::intAdd(z, JavaArithmetic::floatToInt(1.5f + MathHelper::sin(angle) * (float)step));
            setBlockAndMetadata(world, branchX, JavaArithmetic::intAdd(JavaArithmetic::intSub(branchY, 3), step / 2), branchZ,
                                Block::wood->blockID, woodMetadata);
        }
    }

    for (int_t trunkY = 0; trunkY < height; ++trunkY)
    {
        int_t blockId = world->getBlockId(x, JavaArithmetic::intAdd(y, trunkY), z);
        if (blockId == 0 || blockId == Block::leaves->blockID)
        {
            setBlockAndMetadata(world, x, JavaArithmetic::intAdd(y, trunkY), z, Block::wood->blockID, woodMetadata);
            if (trunkY > 0)
            {
                if (random.nextInt(3) > 0 && world->isAirBlock(JavaArithmetic::intSub(x, 1), JavaArithmetic::intAdd(y, trunkY), z))
                    setBlockAndMetadata(world, JavaArithmetic::intSub(x, 1), JavaArithmetic::intAdd(y, trunkY), z, Block::vine->blockID, 8);
                if (random.nextInt(3) > 0 && world->isAirBlock(x, JavaArithmetic::intAdd(y, trunkY), JavaArithmetic::intSub(z, 1)))
                    setBlockAndMetadata(world, x, JavaArithmetic::intAdd(y, trunkY), JavaArithmetic::intSub(z, 1), Block::vine->blockID, 1);
            }
        }

        if (trunkY >= height - 1)
            continue;

        blockId = world->getBlockId(JavaArithmetic::intAdd(x, 1), JavaArithmetic::intAdd(y, trunkY), z);
        if (blockId == 0 || blockId == Block::leaves->blockID)
        {
            setBlockAndMetadata(world, JavaArithmetic::intAdd(x, 1), JavaArithmetic::intAdd(y, trunkY), z, Block::wood->blockID, woodMetadata);
            if (trunkY > 0)
            {
                if (random.nextInt(3) > 0 && world->isAirBlock(JavaArithmetic::intAdd(x, 2), JavaArithmetic::intAdd(y, trunkY), z))
                    setBlockAndMetadata(world, JavaArithmetic::intAdd(x, 2), JavaArithmetic::intAdd(y, trunkY), z, Block::vine->blockID, 2);
                if (random.nextInt(3) > 0 && world->isAirBlock(JavaArithmetic::intAdd(x, 1), JavaArithmetic::intAdd(y, trunkY), JavaArithmetic::intSub(z, 1)))
                    setBlockAndMetadata(world, JavaArithmetic::intAdd(x, 1), JavaArithmetic::intAdd(y, trunkY), JavaArithmetic::intSub(z, 1), Block::vine->blockID, 1);
            }
        }

        blockId = world->getBlockId(JavaArithmetic::intAdd(x, 1), JavaArithmetic::intAdd(y, trunkY), JavaArithmetic::intAdd(z, 1));
        if (blockId == 0 || blockId == Block::leaves->blockID)
        {
            setBlockAndMetadata(world, JavaArithmetic::intAdd(x, 1), JavaArithmetic::intAdd(y, trunkY), JavaArithmetic::intAdd(z, 1), Block::wood->blockID, woodMetadata);
            if (trunkY > 0)
            {
                if (random.nextInt(3) > 0 && world->isAirBlock(JavaArithmetic::intAdd(x, 2), JavaArithmetic::intAdd(y, trunkY), JavaArithmetic::intAdd(z, 1)))
                    setBlockAndMetadata(world, JavaArithmetic::intAdd(x, 2), JavaArithmetic::intAdd(y, trunkY), JavaArithmetic::intAdd(z, 1), Block::vine->blockID, 2);
                if (random.nextInt(3) > 0 && world->isAirBlock(JavaArithmetic::intAdd(x, 1), JavaArithmetic::intAdd(y, trunkY), JavaArithmetic::intAdd(z, 2)))
                    setBlockAndMetadata(world, JavaArithmetic::intAdd(x, 1), JavaArithmetic::intAdd(y, trunkY), JavaArithmetic::intAdd(z, 2), Block::vine->blockID, 4);
            }
        }

        blockId = world->getBlockId(x, JavaArithmetic::intAdd(y, trunkY), JavaArithmetic::intAdd(z, 1));
        if (blockId == 0 || blockId == Block::leaves->blockID)
        {
            setBlockAndMetadata(world, x, JavaArithmetic::intAdd(y, trunkY), JavaArithmetic::intAdd(z, 1), Block::wood->blockID, woodMetadata);
            if (trunkY > 0)
            {
                if (random.nextInt(3) > 0 && world->isAirBlock(JavaArithmetic::intSub(x, 1), JavaArithmetic::intAdd(y, trunkY), JavaArithmetic::intAdd(z, 1)))
                    setBlockAndMetadata(world, JavaArithmetic::intSub(x, 1), JavaArithmetic::intAdd(y, trunkY), JavaArithmetic::intAdd(z, 1), Block::vine->blockID, 8);
                if (random.nextInt(3) > 0 && world->isAirBlock(x, JavaArithmetic::intAdd(y, trunkY), JavaArithmetic::intAdd(z, 2)))
                    setBlockAndMetadata(world, x, JavaArithmetic::intAdd(y, trunkY), JavaArithmetic::intAdd(z, 2), Block::vine->blockID, 4);
            }
        }
    }

    return true;
}

void WorldGenHugeTrees::generateLeafNode(World *world, int_t x, int_t z, int_t y,
                                         int_t radius, Random &random)
{
    constexpr int_t verticalRadius = 2;
    for (int_t leafY = JavaArithmetic::intSub(y, verticalRadius); leafY <= y; ++leafY)
    {
        const int_t relativeY = JavaArithmetic::intSub(leafY, y);
        const int_t layerRadius = JavaArithmetic::intSub(JavaArithmetic::intAdd(radius, 1), relativeY);
        for (int_t leafX = JavaArithmetic::intSub(x, layerRadius); leafX <= JavaArithmetic::intAdd(JavaArithmetic::intAdd(x, layerRadius), 1); ++leafX)
        {
            const int_t relativeX = JavaArithmetic::intSub(leafX, x);
            for (int_t leafZ = JavaArithmetic::intSub(z, layerRadius); leafZ <= JavaArithmetic::intAdd(JavaArithmetic::intAdd(z, layerRadius), 1); ++leafZ)
            {
                const int_t relativeZ = JavaArithmetic::intSub(leafZ, z);
                if ((relativeX >= 0 || relativeZ >= 0 || JavaArithmetic::intAdd(JavaArithmetic::intMul(relativeX, relativeX), JavaArithmetic::intMul(relativeZ, relativeZ)) <= JavaArithmetic::intMul(layerRadius, layerRadius)) &&
                    (relativeX <= 0 && relativeZ <= 0 || JavaArithmetic::intAdd(JavaArithmetic::intMul(relativeX, relativeX), JavaArithmetic::intMul(relativeZ, relativeZ)) <= JavaArithmetic::intMul(JavaArithmetic::intAdd(layerRadius, 1), JavaArithmetic::intAdd(layerRadius, 1))) &&
                    (random.nextInt(4) != 0 || JavaArithmetic::intAdd(JavaArithmetic::intMul(relativeX, relativeX), JavaArithmetic::intMul(relativeZ, relativeZ)) <= JavaArithmetic::intMul(JavaArithmetic::intSub(layerRadius, 1), JavaArithmetic::intSub(layerRadius, 1))) &&
                    !Block::opaqueCubeLookup[world->getBlockId(leafX, leafY, leafZ)])
                {
                    setBlockAndMetadata(world, leafX, leafY, leafZ,
                                        Block::leaves->blockID, leavesMetadata);
                }
            }
        }
    }
}
