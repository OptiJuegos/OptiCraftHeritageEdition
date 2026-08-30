#pragma once

#include "EntityFX.h"

class Tessellator;
class World;

// net.minecraft.src.EntityHugeExplodeFX
class EntityHugeExplodeFX : public EntityFX
{
public:
	EntityHugeExplodeFX(World *world, double x, double y, double z,
	                    double motionX, double motionY, double motionZ);

	void renderParticle(Tessellator *tessellator, float partialTick,
	                    float rotationX, float rotationXZ, float rotationZ,
	                    float rotationYZ, float rotationXY) override;
	void onUpdate() override;
	int_t getFXLayer() override;

private:
	int_t timeSinceStart;
	int_t maximumTime;
};
