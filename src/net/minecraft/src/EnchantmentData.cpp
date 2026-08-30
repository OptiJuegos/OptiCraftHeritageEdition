#include "EnchantmentData.h"
#include "Enchantment.h"
EnchantmentData::EnchantmentData(Enchantment *enchantment, int level)
    : WeightedRandomChoice(enchantment != nullptr ? enchantment->getWeight() : 0), enchantmentobj(enchantment), enchantmentLevel(level) {}
