#pragma once

#include "EntityFX.h"

class World;
class Item;
class Tessellator;

// net.minecraft.src.EntitySlimeFX
class EntitySlimeFX : public EntityFX
{
public:
	EntitySlimeFX(World *world, double d, double d1, double d2, Item *item);

	int_t getFXLayer() override;
	void renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5) override;
};
