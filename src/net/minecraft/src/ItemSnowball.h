#pragma once

#include "Item.h"

class ItemSnowball : public Item {
public:
    ItemSnowball(int id);

    ItemStack* onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) override;
};
