#pragma once

#include "EntityThrowable.h"

class MovingObjectPosition;
class EntityLiving;

// net.minecraft.src.EntityExpBottle
class EntityExpBottle : public EntityThrowable
{
public:
    explicit EntityExpBottle(World *world);
    EntityExpBottle(World *world, EntityLiving *thrower);
    EntityExpBottle(World *world, double x, double y, double z);

protected:
    float getGravityVelocity() const override;
    float getVelocity() const override;
    float getInaccuracyPitchOffset() const override;
    void onImpact(MovingObjectPosition *hit) override;
};
