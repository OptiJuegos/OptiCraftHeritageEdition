#pragma once
#include "IEnchantmentModifier.h"
class EntityLiving;
class EnchantmentModifierLiving : public IEnchantmentModifier
{
public:
    void calculateModifier(Enchantment *enchantment, int level) override;
    int livingModifier = 0;
    EntityLiving *entityLiving = nullptr;
};
