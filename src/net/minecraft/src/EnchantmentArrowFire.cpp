#include "EnchantmentArrowFire.h"
EnchantmentArrowFire::EnchantmentArrowFire(int id, int weight) : Enchantment(id, weight, EnumEnchantmentType::bow) { setName("arrowFire"); }
int EnchantmentArrowFire::getMinEnchantability(int) const { return 20; }
int EnchantmentArrowFire::getMaxEnchantability(int) const { return 50; }
int EnchantmentArrowFire::getMaxLevel() const { return 1; }
