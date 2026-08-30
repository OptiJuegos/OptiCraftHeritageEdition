#pragma once
#include "Enchantment.h"
class EnchantmentLootBonus : public Enchantment
{
public:
    EnchantmentLootBonus(int id, int weight, EnumEnchantmentType type);
    int getMinEnchantability(int level) const override;
    int getMaxEnchantability(int level) const override;
    int getMaxLevel() const override;
    bool canApplyTogether(const Enchantment *other) const override;
};
