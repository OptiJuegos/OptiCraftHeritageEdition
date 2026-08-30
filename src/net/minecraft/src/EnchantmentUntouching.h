#pragma once
#include "Enchantment.h"
class EnchantmentUntouching : public Enchantment
{
public:
    EnchantmentUntouching(int id, int weight);
    int getMinEnchantability(int level) const override;
    int getMaxEnchantability(int level) const override;
    int getMaxLevel() const override;
    bool canApplyTogether(const Enchantment *other) const override;
};
