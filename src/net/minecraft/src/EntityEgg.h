#pragma once

#include "EntityThrowable.h"

class EntityLiving;
class MovingObjectPosition;

// net.minecraft.src.EntityEgg
class EntityEgg : public EntityThrowable
{
public:
	explicit EntityEgg(World *world);
	EntityEgg(World *world, EntityLiving *thrower);
	EntityEgg(World *world, double x, double y, double z);

	void setEggHeading(double x, double y, double z, float velocity, float inaccuracy) { setThrowableHeading(x, y, z, velocity, inaccuracy); }

protected:
	void onImpact(MovingObjectPosition *hit) override;
};
