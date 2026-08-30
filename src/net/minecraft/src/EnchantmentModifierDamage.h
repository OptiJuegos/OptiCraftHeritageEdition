#pragma once
#include "IEnchantmentModifier.h"
class DamageSource;
class EnchantmentModifierDamage : public IEnchantmentModifier
{
public:
    void calculateModifier(Enchantment *enchantment, int level) override;
    int damageModifier = 0;
    const DamageSource *damageSource = nullptr;
};
