#include "EntityDragonPart.h"

#include "DamageSource.h"
#include "EntityDragonBase.h"

EntityDragonPart::EntityDragonPart(EntityDragonBase *dragon, const jstring &partName, float partWidth, float partHeight)
    : Entity(dragon->worldObj), entityDragonObj(dragon), name(partName)
{
    ensureEntityInit();
    setSize(partWidth, partHeight);
}

void EntityDragonPart::entityInit()
{
}

void EntityDragonPart::readEntityFromNBT(NBTTagCompound *)
{
}

void EntityDragonPart::writeEntityToNBT(NBTTagCompound *)
{
}

bool EntityDragonPart::canBeCollidedWith()
{
    return true;
}

bool EntityDragonPart::attackEntityFrom(const DamageSource &source, int_t damage)
{
    return entityDragonObj->attackEntityFromPart(this, source, damage);
}

bool EntityDragonPart::isEntityEqual(Entity *entity)
{
    return this == entity || entityDragonObj == entity;
}
