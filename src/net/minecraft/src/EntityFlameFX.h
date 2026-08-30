#pragma once

#include "EntityFX.h"

class World;
class Tessellator;

// net.minecraft.src.EntityFlameFX
class EntityFlameFX : public EntityFX
{
public:
	EntityFlameFX(World *world, double d, double d1, double d2,
	              double d3, double d4, double d5);

	void renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5) override;
	int_t getBrightnessForRender(float partialTick) override;
	float getEntityBrightness(float f) override;
	void onUpdate() override;

private:
	float field_672_a;
};
