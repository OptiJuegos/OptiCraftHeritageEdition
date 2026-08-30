#pragma once

#include "Item.h"

// net.minecraft.src.ItemGlassBottle
class ItemGlassBottle : public Item
{
public:
    explicit ItemGlassBottle(int id);
    ItemStack *onItemRightClick(ItemStack *itemstack, World *world, EntityPlayer *player) override;
};
