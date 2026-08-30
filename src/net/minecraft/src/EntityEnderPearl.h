#pragma once

#include "EntityThrowable.h"

class MovingObjectPosition;
class EntityLiving;

// net.minecraft.src.EntityEnderPearl
class EntityEnderPearl : public EntityThrowable
{
public:
    explicit EntityEnderPearl(World *world);
    EntityEnderPearl(World *world, EntityLiving *thrower);
    EntityEnderPearl(World *world, double x, double y, double z);

protected:
    void onImpact(MovingObjectPosition *hit) override;
};
