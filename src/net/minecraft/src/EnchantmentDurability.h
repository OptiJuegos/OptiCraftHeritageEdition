#pragma once
#include "Enchantment.h"
class EnchantmentDurability : public Enchantment
{
public:
    EnchantmentDurability(int id, int weight);
    int getMinEnchantability(int level) const override;
    int getMaxEnchantability(int level) const override;
    int getMaxLevel() const override;
};
