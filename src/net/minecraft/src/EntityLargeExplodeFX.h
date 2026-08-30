#pragma once

#include "EntityFX.h"

class RenderEngine;
class Tessellator;
class World;

// net.minecraft.src.EntityLargeExplodeFX
class EntityLargeExplodeFX : public EntityFX
{
public:
	EntityLargeExplodeFX(RenderEngine *renderEngine, World *world,
	                     double x, double y, double z,
	                     double strength, double motionY, double motionZ);

	void renderParticle(Tessellator *tessellator, float partialTick,
	                    float rotationX, float rotationXZ, float rotationZ,
	                    float rotationYZ, float rotationXY) override;
	int_t getBrightnessForRender(float partialTick) override;
	void onUpdate() override;
	int_t getFXLayer() override;

private:
	int_t age;
	int_t maxAge;
	RenderEngine *renderEngine;
	float explosionScale;
};
