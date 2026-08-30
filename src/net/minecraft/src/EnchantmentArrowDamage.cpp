#include "EnchantmentArrowDamage.h"
EnchantmentArrowDamage::EnchantmentArrowDamage(int id, int weight) : Enchantment(id, weight, EnumEnchantmentType::bow) { setName("arrowDamage"); }
int EnchantmentArrowDamage::getMinEnchantability(int level) const { return 1 + (level - 1) * 10; }
int EnchantmentArrowDamage::getMaxEnchantability(int level) const { return getMinEnchantability(level) + 15; }
int EnchantmentArrowDamage::getMaxLevel() const { return 5; }
