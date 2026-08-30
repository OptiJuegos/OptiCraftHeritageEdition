#pragma once

#include "Item.h"

class Block;

class ItemReed : public Item {
public:
    ItemReed(int id, Block* block);

    bool onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) override;

private:
    int itemID; // blockID of the reed block
};
