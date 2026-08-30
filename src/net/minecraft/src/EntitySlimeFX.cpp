#include "EntitySlimeFX.h"

#include "Block.h"
#include "Item.h"
#include "Tessellator.h"
#include "World.h"

EntitySlimeFX::EntitySlimeFX(World *world, double d, double d1, double d2, Item *item)
	: EntityFX(world, d, d1, d2, 0.0, 0.0, 0.0)
{
	particleTextureIndex = item->getIconFromDamage(0);
	particleRed = particleGreen = particleBlue = 1.0f;
	particleGravity = Block::blockSnow->blockParticleGravity;
	particleScale /= 2.0f;
}

int_t EntitySlimeFX::getFXLayer()
{
	return 2;
}

void EntitySlimeFX::renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5)
{
	float f6  = ((float)(particleTextureIndex % 16) + particleTextureJitterX / 4.0f) / 16.0f;
	float f7  = f6 + 0.01560938f;
	float f8  = ((float)(particleTextureIndex / 16) + particleTextureJitterY / 4.0f) / 16.0f;
	float f9  = f8 + 0.01560938f;
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
