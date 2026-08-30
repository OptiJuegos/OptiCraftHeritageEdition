#include "ItemSeeds.h"
#include "ItemStack.h"
#include "EntityPlayer.h"
#include "World.h"
#include "Block.h"

ItemSeeds::ItemSeeds(int id, int cropBlockId)
    : ItemSeeds(id, cropBlockId, Block::tilledField->blockID) {
}

ItemSeeds::ItemSeeds(int id, int cropBlockId, int soilBlockId)
    : Item(id), blockType(cropBlockId), soilBlockID(soilBlockId) {
}

bool ItemSeeds::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    if (l != 1) {
        return false;
    }
    if (!entityplayer->canPlayerEdit(i, j, k) || !entityplayer->canPlayerEdit(i, j + 1, k)) {
        return false;
    }
    int i1 = world->getBlockId(i, j, k);
    if (i1 == soilBlockID && world->isAirBlock(i, j + 1, k)) {
        world->setBlockWithNotify(i, j + 1, k, blockType);
        itemstack->stackSize--;
        return true;
    } else {
        return false;
    }
}
