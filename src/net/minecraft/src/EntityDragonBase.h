#pragma once

#include "EntityLiving.h"

class EntityDragonPart;
class DamageSource;

// net.minecraft.src.EntityDragonBase
class EntityDragonBase : public EntityLiving
{
public:
    explicit EntityDragonBase(World *world, int_t maxHealth = 100);

    int_t getMaxHealth() const override;
    virtual bool attackEntityFromPart(EntityDragonPart *part, const DamageSource &source, int_t damage);
    bool attackEntityFrom(const DamageSource &source, int_t damage) override;

protected:
    bool superAttackFrom(const DamageSource &source, int_t damage);
    int_t dragonMaxHealth;
};
