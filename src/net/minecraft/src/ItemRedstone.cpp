#include "ItemRedstone.h"
#include "ItemStack.h"
#include "World.h"
#include "Block.h"
#include "EntityPlayer.h"

ItemRedstone::ItemRedstone(int i)
    : Item(i) {
}

bool ItemRedstone::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    if (world->getBlockId(i, j, k) != Block::snow->blockID) {
        if (l == 0) j--;
        if (l == 1) j++;
        if (l == 2) k--;
        if (l == 3) k++;
        if (l == 4) i--;
        if (l == 5) i++;
        if (!world->isAirBlock(i, j, k)) {
            return false;
        }
    }
    if (!entityplayer->canPlayerEdit(i, j, k)) {
        return false;
    }
    if (Block::redstoneWire->canPlaceBlockAt(world, i, j, k)) {
        itemstack->stackSize--;
        world->setBlockWithNotify(i, j, k, Block::redstoneWire->blockID);
    }
    return true;
}
