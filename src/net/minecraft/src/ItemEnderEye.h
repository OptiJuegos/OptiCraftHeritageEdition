#pragma once

#include "Item.h"

// net.minecraft.src.ItemEnderEye
class ItemEnderEye : public Item
{
public:
    explicit ItemEnderEye(int id);
    bool onItemUse(ItemStack *stack, EntityPlayer *player, World *world, int x, int y, int z, int side) override;
    ItemStack *onItemRightClick(ItemStack *stack, World *world, EntityPlayer *player) override;
};
