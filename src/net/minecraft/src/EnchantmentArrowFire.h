#pragma once
#include "Enchantment.h"
class EnchantmentArrowFire : public Enchantment
{
public:
    EnchantmentArrowFire(int id, int weight);
    int getMinEnchantability(int level) const override;
    int getMaxEnchantability(int level) const override;
    int getMaxLevel() const override;
};
