#include "ItemSword.h"
#include "ItemStack.h"
#include "Block.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "World.h"

ItemSword::ItemSword(int i, EnumToolMaterial enumtoolmaterial)
    : Item(i) {
    toolMaterial = enumtoolmaterial;
    maxStackSize = 1;
    setMaxDamage(EnumToolMaterialHelper::getMaxUses(enumtoolmaterial));
    weaponDamage = 4 + EnumToolMaterialHelper::getDamageVsEntity(enumtoolmaterial);
}

float ItemSword::getStrVsBlock(ItemStack* itemstack, Block* block) {
    return block->blockID != Block::web->blockID ? 1.5f : 15.0f;
}

bool ItemSword::hitEntity(ItemStack* itemstack, EntityLiving* entityliving, EntityLiving* entityliving1) {
    itemstack->damageItem(1, entityliving1);
    return true;
}

bool ItemSword::onBlockDestroyed(ItemStack* itemstack, int i, int j, int k, int l, EntityLiving* entityliving) {
    itemstack->damageItem(2, entityliving);
    return true;
}

int ItemSword::getDamageVsEntity(Entity* entity) {
    return weaponDamage;
}

bool ItemSword::isFull3D() {
    return true;
}

bool ItemSword::canHarvestBlock(Block* block) {
    return block->blockID == Block::web->blockID;
}

EnumAction ItemSword::getItemUseAction(ItemStack*) { return EnumAction::block; }
int ItemSword::getMaxItemUseDuration(ItemStack*) { return 72000; }
ItemStack* ItemSword::onItemRightClick(ItemStack* itemstack, World*, EntityPlayer* entityplayer) { entityplayer->setItemInUse(itemstack, getMaxItemUseDuration(itemstack)); return itemstack; }
int ItemSword::getItemEnchantability() { return EnumToolMaterialHelper::getEnchantability(toolMaterial); }
