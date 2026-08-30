#pragma once

#include "Item.h"
#include <string>

class ItemDye : public Item {
public:
    ItemDye(int id);

    int getIconFromDamage(int damage) override;
    std::string getItemNameIS(ItemStack* itemstack) override;
    bool onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) override;
    void useItemOnEntity(ItemStack* itemstack, EntityLiving* entityliving) override;
    void saddleEntity(ItemStack* itemstack, EntityLiving* entityliving) override;

    static const std::string dyeColors[16];
    static const int worldChunkManager[16];
};
