#pragma once

#include "EntityThrowable.h"

// net.minecraft.src.EntityPotion
class EntityPotion : public EntityThrowable
{
public:
    explicit EntityPotion(World *world);
    EntityPotion(World *world, EntityLiving *thrower, int damage);
    EntityPotion(World *world, double x, double y, double z, int damage);

    int getPotionDamage() const;

protected:
    float getGravityVelocity() const override;
    float getVelocity() const override;
    float getInaccuracyPitchOffset() const override;
    void onImpact(MovingObjectPosition *hit) override;

private:
    int potionDamage = 0;
};
