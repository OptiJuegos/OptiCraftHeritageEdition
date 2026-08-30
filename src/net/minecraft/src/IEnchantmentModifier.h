#pragma once
class Enchantment;
class IEnchantmentModifier
{
public:
    virtual ~IEnchantmentModifier() = default;
    virtual void calculateModifier(Enchantment *enchantment, int level) = 0;
};
