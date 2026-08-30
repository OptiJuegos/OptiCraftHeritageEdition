#include "EntitySmokeFX.h"
#include "java/Math.h"

#include <cstdlib>

#include "Tessellator.h"
#include "World.h"

static inline double mathRandom_smoke()
{
	return Math::random();
}

EntitySmokeFX::EntitySmokeFX(World *world, double d, double d1, double d2,
                             double d3, double d4, double d5)
	: EntitySmokeFX(world, d, d1, d2, d3, d4, d5, 1.0f)
{
}

EntitySmokeFX::EntitySmokeFX(World *world, double d, double d1, double d2,
                             double d3, double d4, double d5, float f)
	: EntityFX(world, d, d1, d2, 0.0, 0.0, 0.0)
{
	motionX *= 0.10000000149011612;
	motionY *= 0.10000000149011612;
	motionZ *= 0.10000000149011612;
	motionX += d3;
	motionY += d4;
	motionZ += d5;
	particleRed = particleGreen = particleBlue = static_cast<float>(mathRandom_smoke()) * 0.3f;
	particleScale *= 0.75f;
	particleScale *= f;
	field_671_a = particleScale;
	particleMaxAge = (int_t)(8.0 / (mathRandom_smoke() * 0.8 + 0.2));
	particleMaxAge = (int_t)((float)particleMaxAge * f);
	noClip = false;
}

void EntitySmokeFX::renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5)
{
	float f6 = (((float)particleAge + f) / (float)particleMaxAge) * 32.0f;
	if (f6 < 0.0f) { f6 = 0.0f; }
	if (f6 > 1.0f) { f6 = 1.0f; }
	particleScale = field_671_a * f6;
	EntityFX::renderParticle(tessellator, f, f1, f2, f3, f4, f5);
}

void EntitySmokeFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	if (particleAge++ >= particleMaxAge)
	{
		setEntityDead();
	}
	particleTextureIndex = 7 - (particleAge * 8) / particleMaxAge;
	motionY += 0.004;
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
