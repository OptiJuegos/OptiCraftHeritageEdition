#pragma once

#include "Item.h"

class ItemEnderPearl : public Item
{
public:
    explicit ItemEnderPearl(int id);
    ItemStack *onItemRightClick(ItemStack *stack, World *world, EntityPlayer *player) override;
};
