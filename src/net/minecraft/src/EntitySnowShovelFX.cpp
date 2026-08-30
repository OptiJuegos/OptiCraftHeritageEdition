#include "EntitySnowShovelFX.h"
#include "java/Math.h"

#include <cstdlib>

#include "Tessellator.h"
#include "World.h"

static inline double mathRandom_snowshovel()
{
	return Math::random();
}

EntitySnowShovelFX::EntitySnowShovelFX(World *world, double d, double d1, double d2,
                                       double d3, double d4, double d5)
	: EntitySnowShovelFX(world, d, d1, d2, d3, d4, d5, 1.0f)
{
}

EntitySnowShovelFX::EntitySnowShovelFX(World *world, double d, double d1, double d2,
                                       double d3, double d4, double d5, float f)
	: EntityFX(world, d, d1, d2, d3, d4, d5)
{
	motionX *= 0.10000000149011612;
	motionY *= 0.10000000149011612;
	motionZ *= 0.10000000149011612;
	motionX += d3;
	motionY += d4;
	motionZ += d5;
	particleRed = particleGreen = particleBlue = 1.0f - static_cast<float>(mathRandom_snowshovel()) * 0.3f;
	particleScale *= 0.75f;
	particleScale *= f;
	field_27017_a = particleScale;
	particleMaxAge = (int_t)(8.0 / (mathRandom_snowshovel() * 0.8 + 0.2));
	particleMaxAge = (int_t)((float)particleMaxAge * f);
	noClip = false;
}

void EntitySnowShovelFX::renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5)
{
	float f6 = (((float)particleAge + f) / (float)particleMaxAge) * 32.0f;
	if (f6 < 0.0f) { f6 = 0.0f; }
	if (f6 > 1.0f) { f6 = 1.0f; }
	particleScale = field_27017_a * f6;
	EntityFX::renderParticle(tessellator, f, f1, f2, f3, f4, f5);
}

void EntitySnowShovelFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	if (particleAge++ >= particleMaxAge)
	{
		setEntityDead();
	}
	particleTextureIndex = 7 - (particleAge * 8) / particleMaxAge;
	motionY -= 0.03;
	moveEntity(motionX, motionY, motionZ);
	motionX *= 0.99000000953674316;
	motionY *= 0.99000000953674316;
	motionZ *= 0.99000000953674316;
	if (onGround)
	{
		motionX *= 0.69999998807907104;
		motionZ *= 0.69999998807907104;
	}
}
