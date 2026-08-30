#pragma once

#include "Entity.h"

class EntityDragonBase;
class DamageSource;
class NBTTagCompound;

// net.minecraft.src.EntityDragonPart
class EntityDragonPart : public Entity
{
public:
    EntityDragonPart(EntityDragonBase *dragon, const jstring &partName, float width, float height);

    bool canBeCollidedWith() override;
    bool attackEntityFrom(const DamageSource &source, int_t damage) override;
    bool isEntityEqual(Entity *entity) override;

    EntityDragonBase *const entityDragonObj;
    const jstring name;

protected:
    void entityInit() override;
    void readEntityFromNBT(NBTTagCompound *tag) override;
    void writeEntityToNBT(NBTTagCompound *tag) override;
};
