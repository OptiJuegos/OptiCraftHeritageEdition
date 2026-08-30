#include "EnchantmentArrowKnockback.h"
EnchantmentArrowKnockback::EnchantmentArrowKnockback(int id, int weight) : Enchantment(id, weight, EnumEnchantmentType::bow) { setName("arrowKnockback"); }
int EnchantmentArrowKnockback::getMinEnchantability(int level) const { return 12 + (level - 1) * 20; }
int EnchantmentArrowKnockback::getMaxEnchantability(int level) const { return getMinEnchantability(level) + 25; }
int EnchantmentArrowKnockback::getMaxLevel() const { return 2; }
