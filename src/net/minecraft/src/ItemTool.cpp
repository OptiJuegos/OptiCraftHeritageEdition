#include "ItemTool.h"
#include "ItemStack.h"
#include "Block.h"
#include "EntityLiving.h"

ItemTool::ItemTool(int i, int j, EnumToolMaterial enumtoolmaterial, Block** ablock, int numBlocks)
    : Item(i) {
    efficiencyOnProperMaterial = 4.0f;
    toolMaterial = enumtoolmaterial;
    blocksEffectiveAgainst = ablock;
    numBlocksEffectiveAgainst = numBlocks;
    maxStackSize = 1;
    setMaxDamage(EnumToolMaterialHelper::getMaxUses(enumtoolmaterial));
    efficiencyOnProperMaterial = EnumToolMaterialHelper::getEfficiencyOnProperMaterial(enumtoolmaterial);
    damageVsEntity = j + EnumToolMaterialHelper::getDamageVsEntity(enumtoolmaterial);
}

float ItemTool::getStrVsBlock(ItemStack* itemstack, Block* block) {
    for (int i = 0; i < numBlocksEffectiveAgainst; i++) {
        if (blocksEffectiveAgainst[i] == block) {
            return efficiencyOnProperMaterial;
        }
    }
    return 1.0f;
}

bool ItemTool::hitEntity(ItemStack* itemstack, EntityLiving* entityliving, EntityLiving* entityliving1) {
    itemstack->damageItem(2, entityliving1);
    return true;
}

bool ItemTool::onBlockDestroyed(ItemStack* itemstack, int i, int j, int k, int l, EntityLiving* entityliving) {
    itemstack->damageItem(1, entityliving);
    return true;
}

int ItemTool::getDamageVsEntity(Entity* entity) {
    return damageVsEntity;
}

bool ItemTool::isFull3D() {
    return true;
}

int ItemTool::getItemEnchantability() { return EnumToolMaterialHelper::getEnchantability(toolMaterial); }
