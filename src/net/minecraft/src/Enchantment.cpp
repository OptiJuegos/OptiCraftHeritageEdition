#include "Enchantment.h"

#include <stdexcept>

#include "EnchantmentArrowDamage.h"
#include "EnchantmentArrowFire.h"
#include "EnchantmentArrowInfinite.h"
#include "EnchantmentArrowKnockback.h"
#include "EnchantmentDamage.h"
#include "EnchantmentDigging.h"
#include "EnchantmentDurability.h"
#include "EnchantmentFireAspect.h"
#include "EnchantmentKnockback.h"
#include "EnchantmentLootBonus.h"
#include "EnchantmentOxygen.h"
#include "EnchantmentProtection.h"
#include "EnchantmentUntouching.h"
#include "EnchantmentWaterWorker.h"
#include "StatCollector.h"

Enchantment *Enchantment::enchantmentsList[256] = { nullptr };
Enchantment *Enchantment::protection = nullptr;
Enchantment *Enchantment::fireProtection = nullptr;
Enchantment *Enchantment::featherFalling = nullptr;
Enchantment *Enchantment::blastProtection = nullptr;
Enchantment *Enchantment::projectileProtection = nullptr;
Enchantment *Enchantment::respiration = nullptr;
Enchantment *Enchantment::aquaAffinity = nullptr;
Enchantment *Enchantment::sharpness = nullptr;
Enchantment *Enchantment::smite = nullptr;
Enchantment *Enchantment::baneOfArthropods = nullptr;
Enchantment *Enchantment::knockback = nullptr;
Enchantment *Enchantment::fireAspect = nullptr;
Enchantment *Enchantment::looting = nullptr;
Enchantment *Enchantment::efficiency = nullptr;
Enchantment *Enchantment::silkTouch = nullptr;
Enchantment *Enchantment::unbreaking = nullptr;
Enchantment *Enchantment::fortune = nullptr;
Enchantment *Enchantment::power = nullptr;
Enchantment *Enchantment::punch = nullptr;
Enchantment *Enchantment::flame = nullptr;
Enchantment *Enchantment::infinity = nullptr;

Enchantment::Enchantment(int id, int itemWeight, EnumEnchantmentType enchantmentType)
    : effectId(id), type(enchantmentType), weight(itemWeight)
{
    if (id < 0 || id >= 256)
        throw std::invalid_argument("Enchantment id out of range");
    if (enchantmentsList[id] != nullptr)
        throw std::invalid_argument("Duplicate enchantment id");
    enchantmentsList[id] = this;
}

int Enchantment::getWeight() const { return weight; }
int Enchantment::getMinLevel() const { return 1; }
int Enchantment::getMaxLevel() const { return 1; }
int Enchantment::getMinEnchantability(int level) const { return 1 + level * 10; }
int Enchantment::getMaxEnchantability(int level) const { return getMinEnchantability(level) + 5; }
int Enchantment::calcModifierDamage(int, const DamageSource &) const { return 0; }
int Enchantment::calcModifierLiving(int, EntityLiving *) const { return 0; }
bool Enchantment::canApplyTogether(const Enchantment *other) const { return this != other; }
Enchantment *Enchantment::setName(const std::string &value) { name = value; return this; }
std::string Enchantment::getName() const { return "enchantment." + name; }

std::string Enchantment::getTranslatedName(int level) const
{
    return StatCollector::translateToLocal(getName()) + " " +
           StatCollector::translateToLocal("enchantment.level." + std::to_string(level));
}

void Enchantment::initEnchantments()
{
    if (protection != nullptr)
        return;
    protection = (new EnchantmentProtection(0, 10, 0))->setName("protection");
    fireProtection = (new EnchantmentProtection(1, 5, 1))->setName("fireProtection");
    featherFalling = (new EnchantmentProtection(2, 5, 2))->setName("fallProtection");
    blastProtection = (new EnchantmentProtection(3, 2, 3))->setName("explosionProtection");
    projectileProtection = (new EnchantmentProtection(4, 5, 4))->setName("projectileProtection");
    respiration = new EnchantmentOxygen(5, 2);
    aquaAffinity = new EnchantmentWaterWorker(6, 2);
    sharpness = (new EnchantmentDamage(16, 10, 0))->setName("sharpness");
    smite = (new EnchantmentDamage(17, 5, 1))->setName("smite");
    baneOfArthropods = (new EnchantmentDamage(18, 5, 2))->setName("baneOfArthropods");
    knockback = new EnchantmentKnockback(19, 5);
    fireAspect = new EnchantmentFireAspect(20, 2);
    looting = new EnchantmentLootBonus(21, 2, EnumEnchantmentType::weapon);
    efficiency = new EnchantmentDigging(32, 10);
    silkTouch = new EnchantmentUntouching(33, 1);
    unbreaking = new EnchantmentDurability(34, 5);
    fortune = new EnchantmentLootBonus(35, 2, EnumEnchantmentType::digger);
    power = new EnchantmentArrowDamage(48, 10);
    punch = new EnchantmentArrowKnockback(49, 2);
    flame = new EnchantmentArrowFire(50, 2);
    infinity = new EnchantmentArrowInfinite(51, 1);
}

void Enchantment::cleanup()
{
    for (Enchantment *&entry : enchantmentsList)
    {
        delete entry;
        entry = nullptr;
    }
    protection = fireProtection = featherFalling = blastProtection = projectileProtection = nullptr;
    respiration = aquaAffinity = nullptr;
    sharpness = smite = baneOfArthropods = knockback = fireAspect = looting = nullptr;
    efficiency = silkTouch = unbreaking = fortune = nullptr;
    power = punch = flame = infinity = nullptr;
}
