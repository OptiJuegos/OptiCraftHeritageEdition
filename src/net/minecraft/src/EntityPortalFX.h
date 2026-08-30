#pragma once

#include "EntityFX.h"

class World;
class Tessellator;

// net.minecraft.src.EntityPortalFX
class EntityPortalFX : public EntityFX
{
public:
	EntityPortalFX(World *world, double d, double d1, double d2,
	               double d3, double d4, double d5);

	void renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5) override;
	int_t getBrightnessForRender(float partialTick) override;
	float getEntityBrightness(float f) override;
	void onUpdate() override;

private:
	float  field_4083_a;
	double field_4086_p;
	double field_4085_q;
	double field_4084_r;
};
