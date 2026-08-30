#include "EntityPortalFX.h"
#include "java/Math.h"

#include <cstdlib>

#include "Tessellator.h"
#include "World.h"

static inline double mathRandom_portal()
{
	return Math::random();
}

EntityPortalFX::EntityPortalFX(World *world, double d, double d1, double d2,
                               double d3, double d4, double d5)
	: EntityFX(world, d, d1, d2, d3, d4, d5)
{
	motionX = d3;
	motionY = d4;
	motionZ = d5;
	field_4086_p = posX = d;
	field_4085_q = posY = d1;
	field_4084_r = posZ = d2;
	float f = rand.nextFloat() * 0.6f + 0.4f;
	field_4083_a = particleScale = rand.nextFloat() * 0.2f + 0.5f;
	particleRed = particleGreen = particleBlue = 1.0f * f;
	particleGreen *= 0.3f;
	particleRed   *= 0.9f;
	particleMaxAge = (int_t)(mathRandom_portal() * 10.0) + 40;
	noClip = true;
	particleTextureIndex = (int_t)(mathRandom_portal() * 8.0);
}

void EntityPortalFX::renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5)
{
	float f6 = ((float)particleAge + f) / (float)particleMaxAge;
	f6 = 1.0f - f6;
	f6 *= f6;
	f6 = 1.0f - f6;
	particleScale = field_4083_a * f6;
	EntityFX::renderParticle(tessellator, f, f1, f2, f3, f4, f5);
}

int_t EntityPortalFX::getBrightnessForRender(float partialTick)
{
	const int_t packed = EntityFX::getBrightnessForRender(partialTick);
	float age = (float)particleAge / (float)particleMaxAge;
	age *= age;
	age *= age;
	const int_t block = packed & 255;
	int_t sky = packed >> 16 & 255;
	sky += (int_t)(age * 15.0f * 16.0f);
	if (sky > 240) sky = 240;
	return block | sky << 16;
}

float EntityPortalFX::getEntityBrightness(float f)
{
	float f1 = Entity::getEntityBrightness(f);
	float f2 = (float)particleAge / (float)particleMaxAge;
	f2 *= f2;
	f2 *= f2;
	return f1 * (1.0f - f2) + f2;
}

void EntityPortalFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	float f  = (float)particleAge / (float)particleMaxAge;
	float f1 = f;
	f = -f + f * f * 2.0f;
	f = 1.0f - f;
	posX = field_4086_p + motionX * (double)f;
	posY = field_4085_q + motionY * (double)f + (double)(1.0f - f1);
	posZ = field_4084_r + motionZ * (double)f;
	if (particleAge++ >= particleMaxAge)
	{
		setEntityDead();
	}
}
