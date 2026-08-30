#pragma once

#include "ItemFood.h"

class ItemSoup : public ItemFood
{
public:
	ItemSoup(int id, int healAmount);
	ItemStack *onFoodEaten(ItemStack *itemstack, World *world, EntityPlayer *entityplayer) override;
};
