#include "EnchantmentWaterWorker.h"
EnchantmentWaterWorker::EnchantmentWaterWorker(int id, int weight) : Enchantment(id, weight, EnumEnchantmentType::armor_head) { setName("waterWorker"); }
int EnchantmentWaterWorker::getMinEnchantability(int) const { return 1; }
int EnchantmentWaterWorker::getMaxEnchantability(int level) const { return getMinEnchantability(level) + 40; }
int EnchantmentWaterWorker::getMaxLevel() const { return 1; }
