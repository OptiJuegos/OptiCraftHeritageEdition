#pragma once

#include "ItemFood.h"

// net.minecraft.src.ItemAppleGold
class ItemAppleGold : public ItemFood
{
public:
	ItemAppleGold(int id, int healAmount, float saturationModifier, bool wolfFood);
	bool hasEffect(ItemStack *itemstack) override;
	const EnumRarity& getRarity(ItemStack *itemstack) override;
};
