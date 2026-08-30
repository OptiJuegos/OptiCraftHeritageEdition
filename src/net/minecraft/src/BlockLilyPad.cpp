#include "BlockLilyPad.h"

#include "AxisAlignedBB.h"
#include "Block.h"
#include "Material.h"
#include "World.h"

BlockLilyPad::BlockLilyPad(int_t id, int_t texture)
    : BlockFlower(id, texture)
{
    const float radius = 0.5f;
    const float height = 1.0f / 64.0f;
    setBlockBounds(0.5f - radius, 0.0f, 0.5f - radius,
                   0.5f + radius, height, 0.5f + radius);
}

int_t BlockLilyPad::getRenderType()
{
    return 23;
}

AxisAlignedBB *BlockLilyPad::getCollisionBoundingBoxFromPool(World *world, int_t x, int_t y, int_t z)
{
    return AxisAlignedBB::getBoundingBoxFromPool(
        (double)x + minX, (double)y + minY, (double)z + minZ,
        (double)x + maxX, (double)y + maxY, (double)z + maxZ);
}

int_t BlockLilyPad::getBlockColor()
{
    return 2129968;
}

int_t BlockLilyPad::getRenderColor(int_t metadata)
{
    return 2129968;
}

int_t BlockLilyPad::colorMultiplier(IBlockAccess *blockAccess, int_t x, int_t y, int_t z)
{
    return 2129968;
}

bool BlockLilyPad::canThisPlantGrowOnThisBlockID(int_t blockId)
{
    return blockId == Block::waterStill->blockID;
}

bool BlockLilyPad::canBlockStay(World *world, int_t x, int_t y, int_t z)
{
    return y >= 0 && y < 256 &&
           world->getBlockMaterial(x, y - 1, z) == Material::water &&
           world->getBlockMetadata(x, y - 1, z) == 0;
}
