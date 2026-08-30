#pragma once

#include "EntityFX.h"

class World;

// net.minecraft.src.EntitySuspendFX
class EntitySuspendFX : public EntityFX
{
public:
	EntitySuspendFX(World *world, double x, double y, double z,
	                double motionX, double motionY, double motionZ);
	void onUpdate() override;
};
