#include "ItemSlab.h"

#include "AxisAlignedBB.h"
#include "Block.h"
#include "BlockStep.h"
#include "EntityPlayer.h"
#include "ItemStack.h"
#include "StepSound.h"
#include "World.h"

ItemSlab::ItemSlab(int id)
    : ItemBlock(id)
{
    setMaxDamage(0);
    setHasSubtypes(true);
}

int ItemSlab::getIconFromDamage(int damage)
{
    return Block::stairSingle->getBlockTextureFromSideAndMetadata(2, damage);
}

int ItemSlab::getMetadata(int damage)
{
    return damage;
}

std::string ItemSlab::getItemNameIS(ItemStack* itemstack)
{
    int damage = itemstack->getItemDamage();
    if (damage < 0 || damage >= 6)
    {
        damage = 0;
    }
    return getItemName() + "." + BlockStep::blockStepTypes[damage];
}

bool ItemSlab::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int side)
{
    if (itemstack->stackSize == 0)
    {
        return false;
    }
    if (entityplayer != nullptr && !entityplayer->canPlayerEdit(i, j, k))
    {
        return false;
    }

    const int blockId = world->getBlockId(i, j, k);
    const int metadata = world->getBlockMetadata(i, j, k);
    const int slabType = metadata & 7;
    const bool topHalf = (metadata & 8) != 0;
    if ((side == 1 && !topHalf || side == 0 && topHalf) &&
        blockId == Block::stairSingle->blockID && slabType == itemstack->getItemDamage())
    {
        AxisAlignedBB *bounds = Block::stairDouble->getCollisionBoundingBoxFromPool(world, i, j, k);
        const bool clear = bounds != nullptr && world->checkIfAABBIsClear(bounds);
        if (clear && world->setBlockAndMetadataWithNotify(i, j, k, Block::stairDouble->blockID, slabType))
        {
            world->playSoundEffect((double)((float)i + 0.5f), (double)((float)j + 0.5f), (double)((float)k + 0.5f),
                                   Block::stairDouble->stepSound->getStepSound(),
                                   (Block::stairDouble->stepSound->getVolume() + 1.0f) / 2.0f,
                                   Block::stairDouble->stepSound->getPitch() * 0.8f);
            --itemstack->stackSize;
        }
        return true;
    }

    if (tryPlaceDoubleSlab(itemstack, entityplayer, world, i, j, k, side))
    {
        return true;
    }
    return ItemBlock::onItemUse(itemstack, entityplayer, world, i, j, k, side);
}

bool ItemSlab::tryPlaceDoubleSlab(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int side)
{
    (void)entityplayer;
    if (side == 0) --j;
    if (side == 1) ++j;
    if (side == 2) --k;
    if (side == 3) ++k;
    if (side == 4) --i;
    if (side == 5) ++i;

    const int blockId = world->getBlockId(i, j, k);
    const int metadata = world->getBlockMetadata(i, j, k);
    const int slabType = metadata & 7;
    if (blockId != Block::stairSingle->blockID || slabType != itemstack->getItemDamage())
    {
        return false;
    }

    AxisAlignedBB *bounds = Block::stairDouble->getCollisionBoundingBoxFromPool(world, i, j, k);
    const bool clear = bounds != nullptr && world->checkIfAABBIsClear(bounds);
    if (clear && world->setBlockAndMetadataWithNotify(i, j, k, Block::stairDouble->blockID, slabType))
    {
        world->playSoundEffect((double)((float)i + 0.5f), (double)((float)j + 0.5f), (double)((float)k + 0.5f),
                               Block::stairDouble->stepSound->getStepSound(),
                               (Block::stairDouble->stepSound->getVolume() + 1.0f) / 2.0f,
                               Block::stairDouble->stepSound->getPitch() * 0.8f);
        --itemstack->stackSize;
    }
    return true;
}
