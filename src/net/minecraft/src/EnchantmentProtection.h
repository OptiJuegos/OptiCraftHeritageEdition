#pragma once
#include "Enchantment.h"
class EnchantmentProtection : public Enchantment
{
public:
    EnchantmentProtection(int id, int weight, int protectionType);
    int getMinEnchantability(int level) const override;
    int getMaxEnchantability(int level) const override;
    int getMaxLevel() const override;
    int calcModifierDamage(int level, const DamageSource &source) const override;
    std::string getName() const override;
    bool canApplyTogether(const Enchantment *other) const override;
    const int protectionType;
};
