#pragma once

#include "Item.h"

class ItemBoat : public Item {
public:
    ItemBoat(int id);

    ItemStack* onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) override;
};
