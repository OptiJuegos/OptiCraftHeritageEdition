#pragma once

#include "EntityThrowable.h"

class EntityLiving;
class MovingObjectPosition;

// net.minecraft.src.EntitySnowball
class EntitySnowball : public EntityThrowable
{
public:
	explicit EntitySnowball(World *world);
	EntitySnowball(World *world, EntityLiving *thrower);
	EntitySnowball(World *world, double x, double y, double z);

	void setSnowballHeading(double x, double y, double z, float velocity, float inaccuracy) { setThrowableHeading(x, y, z, velocity, inaccuracy); }

protected:
	void onImpact(MovingObjectPosition *hit) override;
};
