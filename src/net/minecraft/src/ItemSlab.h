#pragma once

#include "ItemBlock.h"

class ItemSlab : public ItemBlock {
public:
    ItemSlab(int id);

    int getIconFromDamage(int damage) override;
    int getMetadata(int damage) override;
    std::string getItemNameIS(ItemStack* itemstack) override;
    bool onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int side) override;

private:
    static bool tryPlaceDoubleSlab(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int side);
};
