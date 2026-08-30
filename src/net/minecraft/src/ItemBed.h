#pragma once

#include "Item.h"

class ItemBed : public Item {
public:
    ItemBed(int id);

    bool onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) override;
};
