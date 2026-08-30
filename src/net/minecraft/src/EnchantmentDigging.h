#pragma once
#include "Enchantment.h"
class EnchantmentDigging : public Enchantment
{
public:
    EnchantmentDigging(int id, int weight);
    int getMinEnchantability(int level) const override;
    int getMaxEnchantability(int level) const override;
    int getMaxLevel() const override;
};
