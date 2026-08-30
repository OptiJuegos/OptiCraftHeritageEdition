#include "BlockMushroomCap.h"

#include <algorithm>

#include "Block.h"
#include "BlockFlower.h"
#include "java/Random.h"

BlockMushroomCap::BlockMushroomCap(int_t id, Material *material, int_t texture, int_t type)
    : Block(id, texture, material),
      mushroomType(type)
{
}

int_t BlockMushroomCap::getBlockTextureFromSideAndMetadata(int_t side, int_t metadata)
{
    if (metadata == 10 && side > 1)
        return blockIndexInTexture - 1;
    if (metadata >= 1 && metadata <= 9 && side == 1)
        return blockIndexInTexture - 16 - mushroomType;
    if (metadata >= 1 && metadata <= 3 && side == 2)
        return blockIndexInTexture - 16 - mushroomType;
    if (metadata >= 7 && metadata <= 9 && side == 3)
        return blockIndexInTexture - 16 - mushroomType;
    if ((metadata == 1 || metadata == 4 || metadata == 7) && side == 4)
        return blockIndexInTexture - 16 - mushroomType;
    if ((metadata == 3 || metadata == 6 || metadata == 9) && side == 5)
        return blockIndexInTexture - 16 - mushroomType;
    if (metadata == 14)
        return blockIndexInTexture - 16 - mushroomType;
    if (metadata == 15)
        return blockIndexInTexture - 1;
    return blockIndexInTexture;
}

int_t BlockMushroomCap::quantityDropped(Random &random)
{
    return std::max(0, random.nextInt(10) - 7);
}

int_t BlockMushroomCap::idDropped(int_t metadata, Random &random)
{
    return Block::mushroomBrown->blockID + mushroomType;
}
