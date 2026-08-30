#pragma once
#include "Enchantment.h"
class EnchantmentArrowInfinite : public Enchantment
{
public:
    EnchantmentArrowInfinite(int id, int weight);
    int getMinEnchantability(int level) const override;
    int getMaxEnchantability(int level) const override;
    int getMaxLevel() const override;
};
