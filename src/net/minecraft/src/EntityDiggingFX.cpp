#include "EntityDiggingFX.h"

#include "Block.h"
#include "Tessellator.h"
#include "World.h"

EntityDiggingFX::EntityDiggingFX(World *world, double d, double d1, double d2,
                                 double d3, double d4, double d5,
                                 Block *block, int_t i, int_t j)
	: EntityFX(world, d, d1, d2, d3, d4, d5)
{
	field_32001_o = 0;
	field_4082_a = block;
	particleTextureIndex = block->getBlockTextureFromSideAndMetadata(0, j);
	particleGravity = block->blockParticleGravity;
	particleRed = particleGreen = particleBlue = 0.6f;
	particleScale /= 2.0f;
	field_32001_o = i;
}

EntityDiggingFX *EntityDiggingFX::applyColourMultiplier(int_t i, int_t j, int_t k)
{
	if (field_4082_a == (Block *)Block::grass)
	{
		return this;
	}
	int_t l = field_4082_a->colorMultiplier(worldObj, i, j, k);
	particleRed   *= (float)((l >> 16) & 0xff) / 255.0f;
	particleGreen *= (float)((l >> 8)  & 0xff) / 255.0f;
	particleBlue  *= (float)( l        & 0xff) / 255.0f;
	return this;
}

int_t EntityDiggingFX::getFXLayer()
{
	return 1;
}

void EntityDiggingFX::renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5)
{
	float f6  = ((float)(particleTextureIndex % 16) + particleTextureJitterX / 4.0f) / 16.0f;
	float f7  = f6 + 0.999f / 64.0f;
	float f8  = ((float)(particleTextureIndex / 16) + particleTextureJitterY / 4.0f) / 16.0f;
	float f9  = f8 + 0.999f / 64.0f;
	float f10 = 0.1f * particleScale;
	float f11 = interpolateRenderCoordinate(prevPosX, posX, interpPosX, f);
	float f12 = interpolateRenderCoordinate(prevPosY, posY, interpPosY, f);
	float f13 = interpolateRenderCoordinate(prevPosZ, posZ, interpPosZ, f);
	float f14 = getEntityBrightness(f);
	tessellator->setColorOpaque_F(f14 * particleRed, f14 * particleGreen, f14 * particleBlue);
	tessellator->addVertexWithUV(f11 - f1 * f10 - f4 * f10, f12 - f2 * f10, f13 - f3 * f10 - f5 * f10, f6, f9);
	tessellator->addVertexWithUV((f11 - f1 * f10) + f4 * f10, f12 + f2 * f10, (f13 - f3 * f10) + f5 * f10, f6, f8);
	tessellator->addVertexWithUV(f11 + f1 * f10 + f4 * f10, f12 + f2 * f10, f13 + f3 * f10 + f5 * f10, f7, f8);
	tessellator->addVertexWithUV((f11 + f1 * f10) - f4 * f10, f12 - f2 * f10, (f13 + f3 * f10) - f5 * f10, f7, f9);
}
