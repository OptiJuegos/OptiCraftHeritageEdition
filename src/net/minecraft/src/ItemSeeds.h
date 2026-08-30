#pragma once

#include "Item.h"

class ItemSeeds : public Item {
public:
    ItemSeeds(int id, int cropBlockId);
    ItemSeeds(int id, int cropBlockId, int soilBlockId);

    bool onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) override;

private:
    int blockType;
    int soilBlockID;
};
