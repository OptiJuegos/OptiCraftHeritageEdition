#include "BlockDeadBush.h"
#include "Block.h"
#include "EntityPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "Material.h"
#include "StatList.h"
#include "World.h"

BlockDeadBush::BlockDeadBush(int_t i, int_t j) : BlockFlower(i, j, Material::vine)
{
    float f = 0.4f;
    setBlockBounds(0.5f - f, 0.0f, 0.5f - f, 0.5f + f, 0.8f, 0.5f + f);
}

bool BlockDeadBush::canThisPlantGrowOnThisBlockID(int_t i)
{
    return i == Block::sand->blockID;
}

int_t BlockDeadBush::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
    return blockIndexInTexture;
}

int_t BlockDeadBush::idDropped(int_t i, Random &random)
{
    return -1;
}

void BlockDeadBush::harvestBlock(World *world, EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l)
{
    if (!world->multiplayerWorld && entityplayer->getCurrentEquippedItem() != nullptr &&
        entityplayer->getCurrentEquippedItem()->itemID == Item::shears->shiftedIndex)
    {
        entityplayer->addStat(StatList::mineBlockStatArray[blockID], 1);
        dropBlockAsItem_do(world, i, j, k, new ItemStack(Block::deadBush, 1, l));
    }
    else
    {
        BlockFlower::harvestBlock(world, entityplayer, i, j, k, l);
    }
}
