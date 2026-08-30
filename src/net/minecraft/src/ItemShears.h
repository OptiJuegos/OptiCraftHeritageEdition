#pragma once

#include "Item.h"

class ItemShears : public Item {
public:
    ItemShears(int id);

    bool onBlockDestroyed(ItemStack* itemstack, int i, int j, int k, int l, EntityLiving* entityliving) override;
    bool canHarvestBlock(Block* block) override;
    float getStrVsBlock(ItemStack* itemstack, Block* block) override;
};
