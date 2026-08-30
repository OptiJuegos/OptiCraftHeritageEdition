#pragma once

#include "EntityFX.h"

class Item;
class Tessellator;
class World;

// net.minecraft.src.EntityBreakingFX
class EntityBreakingFX : public EntityFX
{
public:
	EntityBreakingFX(World *world, double x, double y, double z, Item *item);
	EntityBreakingFX(World *world, double x, double y, double z,
	                 double motionX, double motionY, double motionZ, Item *item);

	int_t getFXLayer() override;
	void renderParticle(Tessellator *tessellator, float partialTick,
	                    float rotationX, float rotationXZ, float rotationZ,
	                    float rotationYZ, float rotationXY) override;
};
