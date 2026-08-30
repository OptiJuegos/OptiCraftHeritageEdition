#pragma once

#include "EntityFX.h"
#include "java/Type.h"

class Block;
class Tessellator;
class World;

// net.minecraft.src.EntityDiggingFX
class EntityDiggingFX : public EntityFX
{
public:
	EntityDiggingFX(World *world, double d, double d1, double d2,
	                double d3, double d4, double d5,
	                Block *block, int_t i, int_t j);

	EntityDiggingFX *applyColourMultiplier(int_t i, int_t j, int_t k);

	int_t getFXLayer() override;

	void renderParticle(Tessellator *tessellator, float f,
	                    float f1, float f2, float f3,
	                    float f4, float f5) override;

private:
	Block *field_4082_a;
	int_t  field_32001_o;
};
