#include "EntityReddustFX.h"
#include "java/Math.h"

#include <cstdlib>

#include "Tessellator.h"
#include "World.h"

static inline double mathRandom_red()
{
	return Math::random();
}

EntityReddustFX::EntityReddustFX(World *world, double d, double d1, double d2,
                                 float f, float f1, float f2)
	: EntityReddustFX(world, d, d1, d2, 1.0f, f, f1, f2)
{
}

EntityReddustFX::EntityReddustFX(World *world, double d, double d1, double d2,
                                 float f, float f1, float f2, float f3)
	: EntityFX(world, d, d1, d2, 0.0, 0.0, 0.0)
{
	motionX *= 0.10000000149011612;
	motionY *= 0.10000000149011612;
	motionZ *= 0.10000000149011612;
	if (f1 == 0.0f)
	{
		f1 = 1.0f;
	}
	float f4 = (float)mathRandom_red() * 0.4f + 0.6f;
	particleRed   = (static_cast<float>(mathRandom_red()) * 0.2f + 0.8f) * f1 * f4;
	particleGreen = (static_cast<float>(mathRandom_red()) * 0.2f + 0.8f) * f2 * f4;
	particleBlue  = (static_cast<float>(mathRandom_red()) * 0.2f + 0.8f) * f3 * f4;
	particleScale *= 0.75f;
	particleScale *= f;
	field_673_a = particleScale;
	particleMaxAge = (int_t)(8.0 / (mathRandom_red() * 0.8 + 0.2));
	particleMaxAge = (int_t)((float)particleMaxAge * f);
	noClip = false;
}

void EntityReddustFX::renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5)
{
	float f6 = (((float)particleAge + f) / (float)particleMaxAge) * 32.0f;
	if (f6 < 0.0f) { f6 = 0.0f; }
	if (f6 > 1.0f) { f6 = 1.0f; }
	particleScale = field_673_a * f6;
	EntityFX::renderParticle(tessellator, f, f1, f2, f3, f4, f5);
}

void EntityReddustFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	if (particleAge++ >= particleMaxAge)
	{
		setEntityDead();
	}
	particleTextureIndex = 7 - (particleAge * 8) / particleMaxAge;
	moveEntity(motionX, motionY, motionZ);
	if (posY == prevPosY)
	{
		motionX *= 1.1;
		motionZ *= 1.1;
	}
	motionX *= 0.95999997854232788;
	motionY *= 0.95999997854232788;
	motionZ *= 0.95999997854232788;
	if (onGround)
	{
		motionX *= 0.69999998807907104;
		motionZ *= 0.69999998807907104;
	}
}
