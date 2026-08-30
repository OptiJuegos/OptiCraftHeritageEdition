#include "ItemReed.h"
#include "Block.h"
#include "BlockDeadBush.h"
#include "BlockTallGrass.h"
#include "BlockVine.h"
#include "World.h"
#include "ItemStack.h"
#include "StepSound.h"
#include "EntityPlayer.h"

ItemReed::ItemReed(int i, Block* block)
    : Item(i), itemID(block->blockID) {
}

bool ItemReed::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    int_t blockId = world->getBlockId(i, j, k);
    if (blockId == Block::snow->blockID) {
        l = 1;
    } else if (blockId != Block::vine->blockID && blockId != Block::tallGrass->blockID && blockId != Block::deadBush->blockID) {
        if (l == 0) j--;
        if (l == 1) j++;
        if (l == 2) k--;
        if (l == 3) k++;
        if (l == 4) i--;
        if (l == 5) i++;
    }

    if (!entityplayer->canPlayerEdit(i, j, k)) {
        return false;
    }
    if (itemstack->stackSize == 0) {
        return false;
    }

    if (world->canBlockBePlacedAt(itemID, i, j, k, false, l)) {
        Block* block = Block::blocksList[itemID];
        if (world->setBlockWithNotify(i, j, k, itemID)) {
            if (world->getBlockId(i, j, k) == itemID) {
                block->onBlockPlaced(world, i, j, k, l);
                block->onBlockPlacedBy(world, i, j, k, entityplayer);
            }
            world->playSoundEffect((float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f,
                                   block->stepSound->getStepSound(),
                                   (block->stepSound->getVolume() + 1.0f) / 2.0f,
                                   block->stepSound->getPitch() * 0.8f);
            itemstack->stackSize--;
        }
    }
    return true;
}
