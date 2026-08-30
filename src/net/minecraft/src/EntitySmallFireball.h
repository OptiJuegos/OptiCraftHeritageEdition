#pragma once

#include "EntityFireball.h"

class World;
class EntityLiving;
class MovingObjectPosition;

// net.minecraft.src.EntitySmallFireball
class EntitySmallFireball : public EntityFireball
{
public:
	EntitySmallFireball(World *world);
	EntitySmallFireball(World *world, EntityLiving *shooter, double xAcceleration, double yAcceleration, double zAcceleration);
	EntitySmallFireball(World *world, double x, double y, double z, double xAcceleration, double yAcceleration, double zAcceleration);

	static constexpr int_t CLASS_ID = 31;
	int_t getEntityClassID() const override { return CLASS_ID; }

protected:
	void onImpact(MovingObjectPosition *hit) override;

public:
	bool canBeCollidedWith() override;
	bool attackEntityFrom(Entity *entity, int_t damage) override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;
};
