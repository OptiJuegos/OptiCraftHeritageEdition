#include "BlockFlower.h"
#include "Material.h"
#include "World.h"
#include "Block.h"
#include "AxisAlignedBB.h"
#include "BlockGrass.h"

BlockFlower::BlockFlower(int_t i, int_t j) : BlockFlower(i, j, Material::plants)
{
}

BlockFlower::BlockFlower(int_t i, int_t j, Material *material) : Block(i, material)
{
    blockIndexInTexture = j;
    setTickRandomly(true);
    float f = 0.2f;
    setBlockBounds(0.5f - f, 0.0f, 0.5f - f, 0.5f + f, f * 3.0f, 0.5f + f);
}

bool BlockFlower::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
    return Block::canPlaceBlockAt(world, i, j, k) && canThisPlantGrowOnThisBlockID(world->getBlockId(i, j - 1, k));
}

bool BlockFlower::canThisPlantGrowOnThisBlockID(int_t i)
{
    return i == Block::grass->blockID || i == Block::dirt->blockID || i == Block::tilledField->blockID;
}

void BlockFlower::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
    Block::onNeighborBlockChange(world, i, j, k, l);
    getPlantColor(world, i, j, k);
}

void BlockFlower::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
    getPlantColor(world, i, j, k);
}

void BlockFlower::getPlantColor(World *world, int_t i, int_t j, int_t k)
{
    if (!canBlockStay(world, i, j, k))
    {
        dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
        world->setBlockWithNotify(i, j, k, 0);
    }
}

bool BlockFlower::canBlockStay(World *world, int_t i, int_t j, int_t k)
{
    return (world->getFullBlockLightValue(i, j, k) >= 8 || world->canBlockSeeTheSky(i, j, k))
           && canThisPlantGrowOnThisBlockID(world->getBlockId(i, j - 1, k));
}

AxisAlignedBB *BlockFlower::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
    return nullptr;
}

bool BlockFlower::isOpaqueCube() { return false; }
bool BlockFlower::renderAsNormalBlock() { return false; }
int_t BlockFlower::getRenderType() { return 1; }
