#include "EnchantmentModifierDamage.h"
#include "Enchantment.h"
#include "DamageSource.h"
void EnchantmentModifierDamage::calculateModifier(Enchantment *enchantment, int level)
{
    if (enchantment != nullptr && damageSource != nullptr)
        damageModifier += enchantment->calcModifierDamage(level, *damageSource);
}
