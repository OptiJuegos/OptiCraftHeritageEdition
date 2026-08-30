#include "EntitySpellParticleFX.h"

#include "java/Math.h"

#include "java/Arithmetic.h"
EntitySpellParticleFX::EntitySpellParticleFX(World *world, double x, double y, double z,
                                             double motionXIn, double motionYIn, double motionZIn)
	: EntityFX(world, x, y, z, motionXIn, motionYIn, motionZIn), baseSpellTextureIndex(128)
{
	motionY *= static_cast<double>(0.2f);
	if (motionXIn == 0.0 && motionZIn == 0.0)
	{
		motionX *= static_cast<double>(0.1f);
		motionZ *= static_cast<double>(0.1f);
	}
	particleScale *= 12.0f / 16.0f;
	particleMaxAge = JavaArithmetic::doubleToInt(8.0 / (Math::random() * 0.8 + 0.2));
	noClip = false;
}

void EntitySpellParticleFX::renderParticle(Tessellator *tessellator, float partialTick,
                                           float rotationX, float rotationXZ, float rotationZ,
                                           float rotationYZ, float rotationXY)
{
	EntityFX::renderParticle(tessellator, partialTick, rotationX, rotationXZ,
	                         rotationZ, rotationYZ, rotationXY);
}

void EntitySpellParticleFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	if (particleAge++ >= particleMaxAge)
		setEntityDead();
	setParticleTextureIndex(baseSpellTextureIndex + (7 - particleAge * 8 / particleMaxAge));
	motionY += 0.004;
	moveEntity(motionX, motionY, motionZ);
	if (posY == prevPosY)
	{
		motionX *= 1.1;
		motionZ *= 1.1;
	}
	motionX *= static_cast<double>(0.96f);
	motionY *= static_cast<double>(0.96f);
	motionZ *= static_cast<double>(0.96f);
	if (onGround)
	{
		motionX *= static_cast<double>(0.7f);
		motionZ *= static_cast<double>(0.7f);
	}
}

void EntitySpellParticleFX::setBaseSpellTextureIndex(int_t index)
{
	baseSpellTextureIndex = index;
}
