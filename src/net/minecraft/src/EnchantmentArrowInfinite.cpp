#include "EnchantmentArrowInfinite.h"
EnchantmentArrowInfinite::EnchantmentArrowInfinite(int id, int weight) : Enchantment(id, weight, EnumEnchantmentType::bow) { setName("arrowInfinite"); }
int EnchantmentArrowInfinite::getMinEnchantability(int) const { return 20; }
int EnchantmentArrowInfinite::getMaxEnchantability(int) const { return 50; }
int EnchantmentArrowInfinite::getMaxLevel() const { return 1; }
