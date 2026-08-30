#include "EnchantmentModifierLiving.h"
#include "Enchantment.h"
void EnchantmentModifierLiving::calculateModifier(Enchantment *enchantment, int level)
{
    if (enchantment != nullptr)
        livingModifier += enchantment->calcModifierLiving(level, entityLiving);
}
