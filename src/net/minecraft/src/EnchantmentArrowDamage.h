#pragma once
#include "Enchantment.h"
class EnchantmentArrowDamage : public Enchantment
{
public:
    EnchantmentArrowDamage(int id, int weight);
    int getMinEnchantability(int level) const override;
    int getMaxEnchantability(int level) const override;
    int getMaxLevel() const override;
};
