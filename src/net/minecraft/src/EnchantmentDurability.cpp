#include "EnchantmentDurability.h"
EnchantmentDurability::EnchantmentDurability(int id, int weight) : Enchantment(id, weight, EnumEnchantmentType::digger) { setName("durability"); }
int EnchantmentDurability::getMinEnchantability(int level) const { return 5 + (level - 1) * 10; }
int EnchantmentDurability::getMaxEnchantability(int level) const { return Enchantment::getMinEnchantability(level) + 50; }
int EnchantmentDurability::getMaxLevel() const { return 3; }
