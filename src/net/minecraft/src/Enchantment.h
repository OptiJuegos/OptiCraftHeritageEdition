#pragma once

#include <string>

#include "EnumEnchantmentType.h"

class DamageSource;
class EntityLiving;

class Enchantment
{
public:
    Enchantment(int effectId, int weight, EnumEnchantmentType type);
    virtual ~Enchantment() = default;

    int getWeight() const;
    virtual int getMinLevel() const;
    virtual int getMaxLevel() const;
    virtual int getMinEnchantability(int level) const;
    virtual int getMaxEnchantability(int level) const;
    virtual int calcModifierDamage(int level, const DamageSource &source) const;
    virtual int calcModifierLiving(int level, EntityLiving *living) const;
    virtual bool canApplyTogether(const Enchantment *other) const;
    Enchantment *setName(const std::string &name);
    virtual std::string getName() const;
    std::string getTranslatedName(int level) const;

    static void initEnchantments();
    static void cleanup();

    static Enchantment *enchantmentsList[256];
    static Enchantment *protection;
    static Enchantment *fireProtection;
    static Enchantment *featherFalling;
    static Enchantment *blastProtection;
    static Enchantment *projectileProtection;
    static Enchantment *respiration;
    static Enchantment *aquaAffinity;
    static Enchantment *sharpness;
    static Enchantment *smite;
    static Enchantment *baneOfArthropods;
    static Enchantment *knockback;
    static Enchantment *fireAspect;
    static Enchantment *looting;
    static Enchantment *efficiency;
    static Enchantment *silkTouch;
    static Enchantment *unbreaking;
    static Enchantment *fortune;
    static Enchantment *power;
    static Enchantment *punch;
    static Enchantment *flame;
    static Enchantment *infinity;

    const int effectId;
    EnumEnchantmentType type;

protected:
    int weight;
    std::string name;
};
