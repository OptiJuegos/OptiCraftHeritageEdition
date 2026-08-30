#include "EnchantmentLootBonus.h"
EnchantmentLootBonus::EnchantmentLootBonus(int id, int weight, EnumEnchantmentType typeValue) : Enchantment(id, weight, typeValue)
{
    setName(typeValue == EnumEnchantmentType::digger ? "lootBonusDigger" : "lootBonus");
}
int EnchantmentLootBonus::getMinEnchantability(int level) const { return 20 + (level - 1) * 12; }
int EnchantmentLootBonus::getMaxEnchantability(int level) const { return Enchantment::getMinEnchantability(level) + 50; }
int EnchantmentLootBonus::getMaxLevel() const { return 3; }
bool EnchantmentLootBonus::canApplyTogether(const Enchantment *other) const { return Enchantment::canApplyTogether(other) && (silkTouch == nullptr || other == nullptr || other->effectId != silkTouch->effectId); }
