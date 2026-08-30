#include "EnchantmentOxygen.h"
EnchantmentOxygen::EnchantmentOxygen(int id, int weight) : Enchantment(id, weight, EnumEnchantmentType::armor_head) { setName("oxygen"); }
int EnchantmentOxygen::getMinEnchantability(int level) const { return 10 * level; }
int EnchantmentOxygen::getMaxEnchantability(int level) const { return getMinEnchantability(level) + 30; }
int EnchantmentOxygen::getMaxLevel() const { return 3; }
