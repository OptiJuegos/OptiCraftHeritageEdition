#pragma once

#include "Item.h"

class ItemSign : public Item {
public:
    ItemSign(int id);

    bool onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) override;
};
