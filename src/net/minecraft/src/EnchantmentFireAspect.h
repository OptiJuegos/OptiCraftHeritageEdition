#pragma once
#include "Enchantment.h"
class EnchantmentFireAspect : public Enchantment
{
public:
    EnchantmentFireAspect(int id, int weight);
    int getMinEnchantability(int level) const override;
    int getMaxEnchantability(int level) const override;
    int getMaxLevel() const override;
};
