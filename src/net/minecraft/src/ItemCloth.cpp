#include "ItemCloth.h"
#include "ItemStack.h"
#include "Block.h"
#include "BlockCloth.h"
#include "ItemDye.h"

ItemCloth::ItemCloth(int i)
    : ItemBlock(i) {
    setMaxDamage(0);
    setHasSubtypes(true);
}

int ItemCloth::getIconFromDamage(int i) {
    return Block::cloth->getBlockTextureFromSideAndMetadata(2, BlockCloth::getColorFromDamage(i));
}

int ItemCloth::getMetadata(int i) {
    return i;
}

std::string ItemCloth::getItemNameIS(ItemStack* itemstack) {
    return Item::getItemName() + "." + ItemDye::dyeColors[BlockCloth::getColorFromDamage(itemstack->getItemDamage())];
}
