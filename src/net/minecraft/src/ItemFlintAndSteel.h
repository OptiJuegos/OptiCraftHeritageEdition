#pragma once

#include "Item.h"

class ItemFlintAndSteel : public Item {
public:
    ItemFlintAndSteel(int id);

    bool onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) override;
};
