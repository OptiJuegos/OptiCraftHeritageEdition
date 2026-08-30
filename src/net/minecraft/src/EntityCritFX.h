#pragma once

#include "EntityFX.h"

class Tessellator;
class World;

// net.minecraft.src.EntityCritFX
class EntityCritFX : public EntityFX
{
public:
	EntityCritFX(World *world, double x, double y, double z,
	             double motionX, double motionY, double motionZ);
	EntityCritFX(World *world, double x, double y, double z,
	             double motionX, double motionY, double motionZ, float scale);

	void renderParticle(Tessellator *tessellator, float partialTick,
	                    float rotationX, float rotationXZ, float rotationZ,
	                    float rotationYZ, float rotationXY) override;
	void onUpdate() override;

private:
	float initialScale;
};
