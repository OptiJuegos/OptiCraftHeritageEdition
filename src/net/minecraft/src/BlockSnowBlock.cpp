#include "BlockSnowBlock.h"
#include "Material.h"
#include "Item.h"
#include "EnumSkyBlock.h"
#include "World.h"

BlockSnowBlock::BlockSnowBlock(int_t i, int_t j) : Block(i, j, Material::builtSnow)
{
    setTickOnLoad(true);
}

int_t BlockSnowBlock::idDropped(int_t i, Random &random)
{
    return Item::snowball->shiftedIndex;
}

int_t BlockSnowBlock::quantityDropped(Random &random)
{
    return 4;
}

void BlockSnowBlock::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
    if (world->getSavedLightValue(EnumSkyBlock::Block, i, j, k) > 11)
    {
        dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
        world->setBlockWithNotify(i, j, k, 0);
    }
}
