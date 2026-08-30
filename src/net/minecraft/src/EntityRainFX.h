#pragma once

#include "EntityFX.h"

class World;
class Tessellator;

// net.minecraft.src.EntityRainFX
class EntityRainFX : public EntityFX
{
public:
	EntityRainFX(World *world, double d, double d1, double d2);

	void renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5) override;
	void onUpdate() override;
};
