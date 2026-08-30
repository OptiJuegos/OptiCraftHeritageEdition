#pragma once

#include "Item.h"

class ItemMinecart : public Item {
public:
    ItemMinecart(int id, int minecartType);

    bool onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) override;

    int minecartType;
};
