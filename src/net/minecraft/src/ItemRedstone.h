#pragma once

#include "Item.h"

class ItemRedstone : public Item {
public:
    ItemRedstone(int id);

    bool onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) override;
};
