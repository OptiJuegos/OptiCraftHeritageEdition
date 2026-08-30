#pragma once
#include "Enchantment.h"
class EnchantmentArrowKnockback : public Enchantment
{
public:
    EnchantmentArrowKnockback(int id, int weight);
    int getMinEnchantability(int level) const override;
    int getMaxEnchantability(int level) const override;
    int getMaxLevel() const override;
};
