#pragma once

#include "Item.h"
#include "EnumToolMaterial.h"

class ItemTool : public Item {
public:
    ItemTool(int id, int attackDamage, EnumToolMaterial material, Block** blocksEffectiveAgainst, int numBlocks);

    float getStrVsBlock(ItemStack* itemstack, Block* block) override;
    bool hitEntity(ItemStack* itemstack, EntityLiving* entityliving, EntityLiving* entityliving1) override;
    bool onBlockDestroyed(ItemStack* itemstack, int i, int j, int k, int l, EntityLiving* entityliving) override;
    int getDamageVsEntity(Entity* entity) override;
    bool isFull3D() override;
    int getItemEnchantability() override;

protected:
    Block** blocksEffectiveAgainst;
    int numBlocksEffectiveAgainst;
    float efficiencyOnProperMaterial;
    int damageVsEntity;
    EnumToolMaterial toolMaterial;
};
