#pragma once

#include "EntityFX.h"

class World;
class Tessellator;

// net.minecraft.src.EntitySmokeFX
class EntitySmokeFX : public EntityFX
{
public:
	EntitySmokeFX(World *world, double d, double d1, double d2,
	              double d3, double d4, double d5);
	EntitySmokeFX(World *world, double d, double d1, double d2,
	              double d3, double d4, double d5, float f);

	void renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5) override;
	void onUpdate() override;

	float field_671_a;
};
