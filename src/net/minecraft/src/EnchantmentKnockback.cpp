#include "EnchantmentKnockback.h"
EnchantmentKnockback::EnchantmentKnockback(int id, int weight) : Enchantment(id, weight, EnumEnchantmentType::weapon) { setName("knockback"); }
int EnchantmentKnockback::getMinEnchantability(int level) const { return 5 + 20 * (level - 1); }
int EnchantmentKnockback::getMaxEnchantability(int level) const { return Enchantment::getMinEnchantability(level) + 50; }
int EnchantmentKnockback::getMaxLevel() const { return 2; }
