#pragma once

#include "Item.h"

class ItemFireball : public Item
{
public:
    explicit ItemFireball(int id);
    bool onItemUse(ItemStack *stack, EntityPlayer *player, World *world, int x, int y, int z, int side) override;
};
