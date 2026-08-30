#pragma once

#include "Item.h"
#include "EnumToolMaterial.h"

class ItemSword : public Item {
public:
    ItemSword(int id, EnumToolMaterial material);

    float getStrVsBlock(ItemStack* itemstack, Block* block) override;
    bool hitEntity(ItemStack* itemstack, EntityLiving* entityliving, EntityLiving* entityliving1) override;
    bool onBlockDestroyed(ItemStack* itemstack, int i, int j, int k, int l, EntityLiving* entityliving) override;
    int getDamageVsEntity(Entity* entity) override;
    bool isFull3D() override;
    bool canHarvestBlock(Block* block) override;
    EnumAction getItemUseAction(ItemStack* itemstack) override;
    int getMaxItemUseDuration(ItemStack* itemstack) override;
    ItemStack* onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) override;
    int getItemEnchantability() override;

private:
    int weaponDamage;
    EnumToolMaterial toolMaterial;
};
