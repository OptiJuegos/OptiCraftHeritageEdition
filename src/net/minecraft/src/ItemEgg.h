#pragma once

#include "Item.h"

class ItemEgg : public Item {
public:
    ItemEgg(int id);

    ItemStack* onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) override;
};
