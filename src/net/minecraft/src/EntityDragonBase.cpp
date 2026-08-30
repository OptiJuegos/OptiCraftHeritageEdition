#include "EntityDragonBase.h"

#include "DamageSource.h"

EntityDragonBase::EntityDragonBase(World *world, int_t maxHealth)
    : EntityLiving(world), dragonMaxHealth(maxHealth)
{
    setHealth(getMaxHealth());
}

int_t EntityDragonBase::getMaxHealth() const
{
    return dragonMaxHealth;
}

bool EntityDragonBase::attackEntityFromPart(EntityDragonPart *, const DamageSource &source, int_t damage)
{
    return attackEntityFrom(source, damage);
}

bool EntityDragonBase::attackEntityFrom(const DamageSource &, int_t)
{
    return false;
}

bool EntityDragonBase::superAttackFrom(const DamageSource &source, int_t damage)
{
    return EntityLiving::attackEntityFrom(source, damage);
}
