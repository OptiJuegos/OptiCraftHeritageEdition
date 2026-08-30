#include "ItemShears.h"
#include "ItemStack.h"
#include "EntityLiving.h"
#include "Block.h"
#include "BlockLeaves.h"

ItemShears::ItemShears(int i)
    : Item(i) {
    setMaxStackSize(1);
    setMaxDamage(238);
}

bool ItemShears::onBlockDestroyed(ItemStack* itemstack, int i, int j, int k, int l, EntityLiving* entityliving) {
    if (i == Block::leaves->blockID || i == Block::web->blockID) {
        itemstack->damageItem(1, entityliving);
    }
    return Item::onBlockDestroyed(itemstack, i, j, k, l, entityliving);
}

bool ItemShears::canHarvestBlock(Block* block) {
    return block->blockID == Block::web->blockID;
}

float ItemShears::getStrVsBlock(ItemStack* itemstack, Block* block) {
    if (block->blockID == Block::web->blockID || block->blockID == Block::leaves->blockID) {
        return 15.0f;
    }
    if (block->blockID == Block::cloth->blockID) {
        return 5.0f;
    } else {
        return Item::getStrVsBlock(itemstack, block);
    }
}
