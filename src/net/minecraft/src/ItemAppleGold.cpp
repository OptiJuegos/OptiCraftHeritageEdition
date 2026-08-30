#include "ItemAppleGold.h"

ItemAppleGold::ItemAppleGold(int id, int healAmount, float saturationModifier, bool wolfFood) :
	ItemFood(id, healAmount, saturationModifier, wolfFood)
{
}

bool ItemAppleGold::hasEffect(ItemStack *)
{
	return true;
}

const EnumRarity& ItemAppleGold::getRarity(ItemStack *)
{
	return EnumRarity::epic;
}
