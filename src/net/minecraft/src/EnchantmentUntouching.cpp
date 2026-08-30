#include "EnchantmentUntouching.h"
EnchantmentUntouching::EnchantmentUntouching(int id, int weight) : Enchantment(id, weight, EnumEnchantmentType::digger) { setName("untouching"); }
int EnchantmentUntouching::getMinEnchantability(int) const { return 25; }
int EnchantmentUntouching::getMaxEnchantability(int level) const { return Enchantment::getMinEnchantability(level) + 50; }
bool EnchantmentUntouching::canApplyTogether(const Enchantment *other) const { return Enchantment::canApplyTogether(other) && (fortune == nullptr || other == nullptr || other->effectId != fortune->effectId); }
int EnchantmentUntouching::getMaxLevel() const { return 1; }
