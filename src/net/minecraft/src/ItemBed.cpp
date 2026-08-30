#include "ItemBed.h"
#include "ItemStack.h"
#include "Block.h"
#include "BlockBed.h"
#include "EntityPlayer.h"
#include "MathHelper.h"
#include "World.h"

ItemBed::ItemBed(int i)
    : Item(i) {
}

bool ItemBed::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    if (l != 1) {
        return false;
    }
    j++;
    BlockBed* blockbed = (BlockBed*)Block::blockBed;
    int i1 = MathHelper::floor_double((double)((entityplayer->rotationYaw * 4.0f) / 360.0f) + 0.5) & 3;
    byte_t byte0 = 0;
    byte_t byte1 = 0;
    if (i1 == 0) byte1 = 1;
    if (i1 == 1) byte0 = -1;
    if (i1 == 2) byte1 = -1;
    if (i1 == 3) byte0 = 1;
    if (!entityplayer->canPlayerEdit(i, j, k) ||
        !entityplayer->canPlayerEdit(i + byte0, j, k + byte1)) {
        return false;
    }

    if (world->isAirBlock(i, j, k) && world->isAirBlock(i + byte0, j, k + byte1) &&
        world->isBlockNormalCube(i, j - 1, k) && world->isBlockNormalCube(i + byte0, j - 1, k + byte1)) {
        world->setBlockAndMetadataWithNotify(i, j, k, blockbed->blockID, i1);
        if (world->getBlockId(i, j, k) == blockbed->blockID) {
            world->setBlockAndMetadataWithNotify(i + byte0, j, k + byte1, blockbed->blockID, i1 + 8);
        }
        itemstack->stackSize--;
        return true;
    }
    return false;
}
