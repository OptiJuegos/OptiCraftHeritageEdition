#pragma once
#include "Enchantment.h"
class EnchantmentDamage : public Enchantment
{
public:
    EnchantmentDamage(int id, int weight, int damageType);
    int getMinEnchantability(int level) const override;
    int getMaxEnchantability(int level) const override;
    int getMaxLevel() const override;
    int calcModifierLiving(int level, EntityLiving *living) const override;
    std::string getName() const override;
    bool canApplyTogether(const Enchantment *other) const override;
    const int damageType;
};
