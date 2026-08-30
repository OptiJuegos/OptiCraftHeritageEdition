#pragma once

#include "Item.h"

// net.minecraft.src.ItemBucketMilk
class ItemBucketMilk : public Item
{
public:
	explicit ItemBucketMilk(int id);
	ItemStack *onFoodEaten(ItemStack *itemstack, World *world, EntityPlayer *entityplayer) override;
	int getMaxItemUseDuration(ItemStack *itemstack) override;
	EnumAction getItemUseAction(ItemStack *itemstack) override;
	ItemStack *onItemRightClick(ItemStack *itemstack, World *world, EntityPlayer *entityplayer) override;
};
