#include "EntityBreakingFX.h"

#include "Block.h"
#include "Item.h"
#include "Tessellator.h"

EntityBreakingFX::EntityBreakingFX(World *world, double x, double y, double z, Item *item)
	: EntityFX(world, x, y, z, 0.0, 0.0, 0.0)
{
	setParticleTextureIndex(item != nullptr ? item->getIconFromDamage(0) : 0);
	particleRed = particleGreen = particleBlue = 1.0f;
	particleGravity = Block::blockSnow != nullptr ? Block::blockSnow->blockParticleGravity : 0.0f;
	particleScale /= 2.0f;
}

EntityBreakingFX::EntityBreakingFX(World *world, double x, double y, double z,
                                   double motionXIn, double motionYIn, double motionZIn,
                                   Item *item)
	: EntityBreakingFX(world, x, y, z, item)
{
	motionX *= static_cast<double>(0.1f);
	motionY *= static_cast<double>(0.1f);
	motionZ *= static_cast<double>(0.1f);
	motionX += motionXIn;
	motionY += motionYIn;
	motionZ += motionZIn;
}

int_t EntityBreakingFX::getFXLayer()
{
	return 2;
}

void EntityBreakingFX::renderParticle(Tessellator *tessellator, float partialTick,
                                      float rotationX, float rotationXZ, float rotationZ,
                                      float rotationYZ, float rotationXY)
{
	const float u0 = (static_cast<float>(getParticleTextureIndex() % 16) + particleTextureJitterX / 4.0f) / 16.0f;
	const float u1 = u0 + 0.999f / 64.0f;
	const float v0 = (static_cast<float>(getParticleTextureIndex() / 16) + particleTextureJitterY / 4.0f) / 16.0f;
	const float v1 = v0 + 0.999f / 64.0f;
	const float scale = 0.1f * particleScale;
	const float x = interpolateRenderCoordinate(prevPosX, posX, interpPosX, partialTick);
	const float y = interpolateRenderCoordinate(prevPosY, posY, interpPosY, partialTick);
	const float z = interpolateRenderCoordinate(prevPosZ, posZ, interpPosZ, partialTick);

	tessellator->setColorOpaque_F(particleRed, particleGreen, particleBlue);
	tessellator->addVertexWithUV(x - rotationX * scale - rotationYZ * scale, y - rotationXZ * scale,
	                             z - rotationZ * scale - rotationXY * scale, u0, v1);
	tessellator->addVertexWithUV(x - rotationX * scale + rotationYZ * scale, y + rotationXZ * scale,
	                             z - rotationZ * scale + rotationXY * scale, u0, v0);
	tessellator->addVertexWithUV(x + rotationX * scale + rotationYZ * scale, y + rotationXZ * scale,
	                             z + rotationZ * scale + rotationXY * scale, u1, v0);
	tessellator->addVertexWithUV(x + rotationX * scale - rotationYZ * scale, y - rotationXZ * scale,
	                             z + rotationZ * scale - rotationXY * scale, u1, v1);
}
