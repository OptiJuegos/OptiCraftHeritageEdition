#pragma once

#include "EntityFX.h"

class World;
class Tessellator;

// net.minecraft.src.EntityReddustFX
class EntityReddustFX : public EntityFX
{
public:
	EntityReddustFX(World *world, double d, double d1, double d2,
	                float f, float f1, float f2);
	EntityReddustFX(World *world, double d, double d1, double d2,
	                float f, float f1, float f2, float f3);

	void renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5) override;
	void onUpdate() override;

	float field_673_a;
};
