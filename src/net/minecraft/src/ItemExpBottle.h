#pragma once

#include "Item.h"

class ItemExpBottle : public Item
{
public:
    explicit ItemExpBottle(int id);
    bool hasEffect(ItemStack *stack) override;
    ItemStack *onItemRightClick(ItemStack *stack, World *world, EntityPlayer *player) override;
};
