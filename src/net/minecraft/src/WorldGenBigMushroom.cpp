#include "WorldGenBigMushroom.h"

#include "java/Arithmetic.h"
#include "Block.h"
#include "BlockFlower.h"
#include "BlockGrass.h"
#include "BlockLeaves.h"
#include "BlockMushroomCap.h"
#include "BlockMycelium.h"
#include "World.h"

WorldGenBigMushroom::WorldGenBigMushroom()
    : WorldGenerator(false),
      mushroomType(-1)
{
}

WorldGenBigMushroom::WorldGenBigMushroom(int_t type)
    : WorldGenerator(true),
      mushroomType(type)
{
}

bool WorldGenBigMushroom::generate(World *world, Random &random, int_t x, int_t y, int_t z)
{
    int_t type = random.nextInt(2);
    if (mushroomType >= 0)
        type = mushroomType;

    const int_t height = random.nextInt(3) + 4;
    bool clear = true;
    if (y < 1 || JavaArithmetic::intAdd(JavaArithmetic::intAdd(y, height), 1) >= 256)
        return false;

    for (int_t checkY = y; checkY <= JavaArithmetic::intAdd(JavaArithmetic::intAdd(y, 1), height) && clear; ++checkY)
    {
        const int_t radius = checkY == y ? 0 : 3;
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
                if (blockId != 0 && blockId != Block::leaves->blockID)
                    clear = false;
            }
        }
    }

    if (!clear)
        return false;

    const int_t groundId = world->getBlockId(x, JavaArithmetic::intSub(y, 1), z);
    if (groundId != Block::dirt->blockID && groundId != Block::grass->blockID &&
        groundId != Block::mycelium->blockID)
        return false;
    if (!Block::mushroomBrown->canPlaceBlockAt(world, x, y, z))
        return false;

    setBlockAndMetadata(world, x, JavaArithmetic::intSub(y, 1), z, Block::dirt->blockID, 0);
    int_t capBottom = JavaArithmetic::intAdd(y, height);
    if (type == 1)
        capBottom = JavaArithmetic::intSub(JavaArithmetic::intAdd(y, height), 3);

    for (int_t capY = capBottom; capY <= JavaArithmetic::intAdd(y, height); ++capY)
    {
        int_t radius = 1;
        if (capY < JavaArithmetic::intAdd(y, height))
            ++radius;
        if (type == 0)
            radius = 3;

        for (int_t capX = JavaArithmetic::intSub(x, radius); capX <= JavaArithmetic::intAdd(x, radius); ++capX)
        {
            for (int_t capZ = JavaArithmetic::intSub(z, radius); capZ <= JavaArithmetic::intAdd(z, radius); ++capZ)
            {
                int_t metadata = 5;
                if (capX == JavaArithmetic::intSub(x, radius)) --metadata;
                if (capX == JavaArithmetic::intAdd(x, radius)) ++metadata;
                if (capZ == JavaArithmetic::intSub(z, radius)) metadata -= 3;
                if (capZ == JavaArithmetic::intAdd(z, radius)) metadata += 3;

                if (type == 0 || capY < JavaArithmetic::intAdd(y, height))
                {
                    if ((capX == JavaArithmetic::intSub(x, radius) || capX == JavaArithmetic::intAdd(x, radius)) &&
                        (capZ == JavaArithmetic::intSub(z, radius) || capZ == JavaArithmetic::intAdd(z, radius)))
                        continue;
                    if (capX == JavaArithmetic::intSub(x, JavaArithmetic::intSub(radius, 1)) && capZ == JavaArithmetic::intSub(z, radius)) metadata = 1;
                    if (capX == JavaArithmetic::intSub(x, radius) && capZ == JavaArithmetic::intSub(z, JavaArithmetic::intSub(radius, 1))) metadata = 1;
                    if (capX == JavaArithmetic::intAdd(x, JavaArithmetic::intSub(radius, 1)) && capZ == JavaArithmetic::intSub(z, radius)) metadata = 3;
                    if (capX == JavaArithmetic::intAdd(x, radius) && capZ == JavaArithmetic::intSub(z, JavaArithmetic::intSub(radius, 1))) metadata = 3;
                    if (capX == JavaArithmetic::intSub(x, JavaArithmetic::intSub(radius, 1)) && capZ == JavaArithmetic::intAdd(z, radius)) metadata = 7;
                    if (capX == JavaArithmetic::intSub(x, radius) && capZ == JavaArithmetic::intAdd(z, JavaArithmetic::intSub(radius, 1))) metadata = 7;
                    if (capX == JavaArithmetic::intAdd(x, JavaArithmetic::intSub(radius, 1)) && capZ == JavaArithmetic::intAdd(z, radius)) metadata = 9;
                    if (capX == JavaArithmetic::intAdd(x, radius) && capZ == JavaArithmetic::intAdd(z, JavaArithmetic::intSub(radius, 1))) metadata = 9;
                }

                if (metadata == 5 && capY < JavaArithmetic::intAdd(y, height))
                    metadata = 0;

                if ((metadata != 0 || y >= JavaArithmetic::intSub(JavaArithmetic::intAdd(y, height), 1)) &&
                    !Block::opaqueCubeLookup[world->getBlockId(capX, capY, capZ)])
                {
                    setBlockAndMetadata(world, capX, capY, capZ,
                                        JavaArithmetic::intAdd(Block::mushroomCapBrown->blockID, type), metadata);
                }
            }
        }
    }

    for (int_t trunkY = 0; trunkY < height; ++trunkY)
    {
        const int_t blockId = world->getBlockId(x, JavaArithmetic::intAdd(y, trunkY), z);
        if (!Block::opaqueCubeLookup[blockId])
        {
            setBlockAndMetadata(world, x, JavaArithmetic::intAdd(y, trunkY), z,
                                JavaArithmetic::intAdd(Block::mushroomCapBrown->blockID, type), 10);
        }
    }

    return true;
}
