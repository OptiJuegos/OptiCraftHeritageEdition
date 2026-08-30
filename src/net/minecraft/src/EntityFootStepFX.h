#pragma once

#include "EntityFX.h"

class World;
class Tessellator;
class RenderEngine;

// net.minecraft.src.EntityFootStepFX
class EntityFootStepFX : public EntityFX
{
public:
	EntityFootStepFX(RenderEngine *renderengine, World *world, double d, double d1, double d2);

	void renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5) override;
	void onUpdate() override;
	int_t getFXLayer() override;

private:
	int_t field_27018_a;
	int_t field_27020_o;
	RenderEngine *field_27019_p;
};
