#pragma once

#include "Item.h"

class ItemFood : public Item
{
public:
	using Item::setPotionEffect;
	ItemFood(int id, int healAmount, float saturationModifier, bool isWolfsFavoriteMeat);
	ItemFood(int id, int healAmount, bool isWolfsFavoriteMeat);

	ItemStack *onFoodEaten(ItemStack *itemstack, World *world, EntityPlayer *entityplayer) override;
	int getMaxItemUseDuration(ItemStack *itemstack) override;
	EnumAction getItemUseAction(ItemStack *itemstack) override;
	ItemStack *onItemRightClick(ItemStack *itemstack, World *world, EntityPlayer *entityplayer) override;
	int getHealAmount() const;
	float getSaturationModifier() const;
	bool isWolfsFavoriteMeat() const;
	bool isWolfsFavoriteMeatItem() const;
	bool getIsWolfsFavoriteMeat() const;
	ItemFood *setPotionEffect(int potionId, int duration, int amplifier, float probability);
	ItemFood *setAlwaysEdible();

protected:
	int healAmount;
	float saturationModifier;
	bool wolfsFavoriteMeat;
	bool alwaysEdible;
	int potionId;
	int potionDuration;
	int potionAmplifier;
	float potionEffectProbability;
};
