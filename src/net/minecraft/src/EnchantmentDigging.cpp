#include "EnchantmentDigging.h"
EnchantmentDigging::EnchantmentDigging(int id, int weight) : Enchantment(id, weight, EnumEnchantmentType::digger) { setName("digging"); }
int EnchantmentDigging::getMinEnchantability(int level) const { return 1 + 15 * (level - 1); }
int EnchantmentDigging::getMaxEnchantability(int level) const { return Enchantment::getMinEnchantability(level) + 50; }
int EnchantmentDigging::getMaxLevel() const { return 5; }
