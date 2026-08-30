#pragma once
#include "Enchantment.h"
class EnchantmentOxygen : public Enchantment
{
public:
    EnchantmentOxygen(int id, int weight);
    int getMinEnchantability(int level) const override;
    int getMaxEnchantability(int level) const override;
    int getMaxLevel() const override;
};
