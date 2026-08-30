#pragma once
#include "Enchantment.h"
class EnchantmentKnockback : public Enchantment
{
public:
    EnchantmentKnockback(int id, int weight);
    int getMinEnchantability(int level) const override;
    int getMaxEnchantability(int level) const override;
    int getMaxLevel() const override;
};
