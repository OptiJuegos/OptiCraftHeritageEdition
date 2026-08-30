#pragma once

#include "Item.h"

class ItemBow : public Item {
public:
    explicit ItemBow(int id);

    ItemStack* onFoodEaten(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) override;
    void onPlayerStoppedUsing(ItemStack* itemstack, World* world, EntityPlayer* entityplayer, int timeLeft) override;
    int getMaxItemUseDuration(ItemStack* itemstack) override;
    EnumAction getItemUseAction(ItemStack* itemstack) override;
    ItemStack* onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) override;
    int getItemEnchantability() override;
};
