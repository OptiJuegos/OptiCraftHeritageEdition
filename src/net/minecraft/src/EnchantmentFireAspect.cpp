#include "EnchantmentFireAspect.h"
EnchantmentFireAspect::EnchantmentFireAspect(int id, int weight) : Enchantment(id, weight, EnumEnchantmentType::weapon) { setName("fire"); }
int EnchantmentFireAspect::getMinEnchantability(int level) const { return 10 + 20 * (level - 1); }
int EnchantmentFireAspect::getMaxEnchantability(int level) const { return Enchantment::getMinEnchantability(level) + 50; }
int EnchantmentFireAspect::getMaxLevel() const { return 2; }
