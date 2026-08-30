#pragma once

#include "Item.h"

class ItemPainting : public Item {
public:
    ItemPainting(int id);

    bool onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) override;
};
