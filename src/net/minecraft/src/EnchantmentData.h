#pragma once
#include "WeightedRandomChoice.h"
class Enchantment;
class EnchantmentData : public WeightedRandomChoice
{
public:
    EnchantmentData(Enchantment *enchantment, int level);
    Enchantment *enchantmentobj;
    int enchantmentLevel;
};
