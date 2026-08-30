#include "EnchantmentProtection.h"
#include "DamageSource.h"

namespace
{
const char *kProtectionNames[] = { "all", "fire", "fall", "explosion", "projectile" };
const int kBaseEnchantability[] = { 1, 10, 5, 5, 3 };
const int kLevelEnchantability[] = { 16, 8, 6, 8, 6 };
const int kThresholdEnchantability[] = { 20, 12, 10, 12, 15 };
}

EnchantmentProtection::EnchantmentProtection(int id, int weight, int typeValue)
    : Enchantment(id, weight, EnumEnchantmentType::armor), protectionType(typeValue)
{
    if (typeValue == 2)
        type = EnumEnchantmentType::armor_feet;
}
int EnchantmentProtection::getMinEnchantability(int level) const { return kBaseEnchantability[protectionType] + (level - 1) * kLevelEnchantability[protectionType]; }
int EnchantmentProtection::getMaxEnchantability(int level) const { return getMinEnchantability(level) + kThresholdEnchantability[protectionType]; }
int EnchantmentProtection::getMaxLevel() const { return 4; }
int EnchantmentProtection::calcModifierDamage(int level, const DamageSource &source) const
{
    if (source.canHarmInCreative()) return 0;
    const int modifier = (6 + level * level) / 2;
    if (protectionType == 0) return modifier;
    if (protectionType == 1 && source.fireDamage()) return modifier;
    if (protectionType == 2 && &source == &DamageSource::fall) return modifier * 2;
    if (protectionType == 3 && &source == &DamageSource::explosion) return modifier;
    if (protectionType == 4 && source.isProjectile()) return modifier;
    return 0;
}
std::string EnchantmentProtection::getName() const { return std::string("enchantment.protect.") + kProtectionNames[protectionType]; }
bool EnchantmentProtection::canApplyTogether(const Enchantment *other) const
{
    const EnchantmentProtection *protection = dynamic_cast<const EnchantmentProtection *>(other);
    if (protection == nullptr) return Enchantment::canApplyTogether(other);
    if (protection->protectionType == protectionType) return false;
    return protectionType == 2 || protection->protectionType == 2;
}
