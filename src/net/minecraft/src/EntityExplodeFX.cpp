#include "EntityExplodeFX.h"
#include "java/Math.h"

#include <cstdlib>

#include "Tessellator.h"
#include "World.h"

static inline double mathRandom_explode()
{
	return Math::random();
}

EntityExplodeFX::EntityExplodeFX(World *world, double d, double d1, double d2,
                                 double d3, double d4, double d5)
	: EntityFX(world, d, d1, d2, d3, d4, d5)
{
	motionX = d3 + (double)((float)(mathRandom_explode() * 2.0 - 1.0) * 0.05f);
	motionY = d4 + (double)((float)(mathRandom_explode() * 2.0 - 1.0) * 0.05f);
	motionZ = d5 + (double)((float)(mathRandom_explode() * 2.0 - 1.0) * 0.05f);
	particleRed = particleGreen = particleBlue = rand.nextFloat() * 0.3f + 0.7f;
	particleScale = rand.nextFloatProduct() * 6.0f + 1.0f;
	particleMaxAge = (int_t)(16.0 / ((double)rand.nextFloat() * 0.8 + 0.2)) + 2;
}

void EntityExplodeFX::renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5)
{
	EntityFX::renderParticle(tessellator, f, f1, f2, f3, f4, f5);
}

void EntityExplodeFX::onUpdate()
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
	motionX *= 0.89999997615814209;
	motionY *= 0.89999997615814209;
	motionZ *= 0.89999997615814209;
	if (onGround)
	{
		motionX *= 0.69999998807907104;
		motionZ *= 0.69999998807907104;
	}
}
