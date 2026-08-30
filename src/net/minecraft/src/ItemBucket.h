#pragma once

#include "Item.h"

class ItemBucket : public Item {
public:
    ItemBucket(int id, int isFull);

    ItemStack* onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) override;

    int isFull;
};
