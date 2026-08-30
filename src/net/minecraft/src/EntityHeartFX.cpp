#include "EntityHeartFX.h"

#include "Tessellator.h"
#include "World.h"

EntityHeartFX::EntityHeartFX(World *world, double d, double d1, double d2,
                             double d3, double d4, double d5)
	: EntityHeartFX(world, d, d1, d2, d3, d4, d5, 2.0f)
{
}

EntityHeartFX::EntityHeartFX(World *world, double d, double d1, double d2,
                             double d3, double d4, double d5, float f)
	: EntityFX(world, d, d1, d2, 0.0, 0.0, 0.0)
{
	motionX *= 0.0099999997764825821;
	motionY *= 0.0099999997764825821;
	motionZ *= 0.0099999997764825821;
	motionY += 0.1;
	particleScale *= 0.75f;
	particleScale *= f;
	field_25022_a = particleScale;
	particleMaxAge = 16;
	noClip = false;
	particleTextureIndex = 80;
}

void EntityHeartFX::renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5)
{
	float f6 = (((float)particleAge + f) / (float)particleMaxAge) * 32.0f;
	if (f6 < 0.0f) { f6 = 0.0f; }
	if (f6 > 1.0f) { f6 = 1.0f; }
	particleScale = field_25022_a * f6;
	EntityFX::renderParticle(tessellator, f, f1, f2, f3, f4, f5);
}

void EntityHeartFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	if (particleAge++ >= particleMaxAge)
	{
		setEntityDead();
	}
	moveEntity(motionX, motionY, motionZ);
	if (posY == prevPosY)
	{
		motionX *= 1.1;
		motionZ *= 1.1;
	}
	motionX *= 0.86000001430511475;
	motionY *= 0.86000001430511475;
	motionZ *= 0.86000001430511475;
	if (onGround)
	{
		motionX *= 0.69999998807907104;
		motionZ *= 0.69999998807907104;
	}
}
