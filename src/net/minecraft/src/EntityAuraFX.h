#pragma once

#include "EntityFX.h"

class World;

// net.minecraft.src.EntityAuraFX
class EntityAuraFX : public EntityFX
{
public:
	EntityAuraFX(World *world, double x, double y, double z,
	             double motionX, double motionY, double motionZ);
	void onUpdate() override;
};
