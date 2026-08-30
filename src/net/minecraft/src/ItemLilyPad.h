#pragma once

#include "ItemColored.h"

class EntityPlayer;
class ItemStack;
class World;

// net.minecraft.src.ItemLilyPad
class ItemLilyPad : public ItemColored
{
public:
    explicit ItemLilyPad(int id);

    ItemStack *onItemRightClick(ItemStack *itemstack, World *world, EntityPlayer *player) override;
    int getColorFromDamage(int damage, int renderPass) override;
};
