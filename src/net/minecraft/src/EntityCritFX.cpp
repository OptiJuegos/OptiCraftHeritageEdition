#include "EntityCritFX.h"

#include "java/Math.h"

#include "java/Arithmetic.h"
EntityCritFX::EntityCritFX(World *world, double x, double y, double z,
                           double motionXIn, double motionYIn, double motionZIn)
	: EntityCritFX(world, x, y, z, motionXIn, motionYIn, motionZIn, 1.0f)
{
}

EntityCritFX::EntityCritFX(World *world, double x, double y, double z,
                           double motionXIn, double motionYIn, double motionZIn,
                           float scale)
	: EntityFX(world, x, y, z, 0.0, 0.0, 0.0), initialScale(0.0f)
{
	motionX *= static_cast<double>(0.1f);
	motionY *= static_cast<double>(0.1f);
	motionZ *= static_cast<double>(0.1f);
	motionX += motionXIn * 0.4;
	motionY += motionYIn * 0.4;
	motionZ += motionZIn * 0.4;
	const float randomShade = static_cast<float>(Math::random());
	particleRed = particleGreen = particleBlue = randomShade * 0.3f + 0.6f;
	particleScale *= 12.0f / 16.0f;
	particleScale *= scale;
	initialScale = particleScale;
	particleMaxAge = JavaArithmetic::doubleToInt(6.0 / (Math::random() * 0.8 + 0.6));
	particleMaxAge = JavaArithmetic::floatToInt(static_cast<float>(particleMaxAge) * scale);
	noClip = false;
	setParticleTextureIndex(65);
	onUpdate();
}

void EntityCritFX::renderParticle(Tessellator *tessellator, float partialTick,
                                  float rotationX, float rotationXZ, float rotationZ,
                                  float rotationYZ, float rotationXY)
{
	float ageScale = (static_cast<float>(particleAge) + partialTick) /
	                 static_cast<float>(particleMaxAge) * 32.0f;
	if (ageScale < 0.0f)
		ageScale = 0.0f;
	if (ageScale > 1.0f)
		ageScale = 1.0f;
	particleScale = initialScale * ageScale;
	EntityFX::renderParticle(tessellator, partialTick, rotationX, rotationXZ,
	                         rotationZ, rotationYZ, rotationXY);
}

void EntityCritFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	if (particleAge++ >= particleMaxAge)
		setEntityDead();
	moveEntity(motionX, motionY, motionZ);
	particleGreen *= 0.96f;
	particleBlue *= 0.9f;
	motionX *= static_cast<double>(0.7f);
	motionY *= static_cast<double>(0.7f);
	motionZ *= static_cast<double>(0.7f);
	motionY -= static_cast<double>(0.02f);
	if (onGround)
	{
		motionX *= static_cast<double>(0.7f);
		motionZ *= static_cast<double>(0.7f);
	}
}
