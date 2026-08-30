#include "EntityBubbleFX.h"
#include "java/Math.h"

#include <cstdlib>

#include "MathHelper.h"
#include "Material.h"
#include "World.h"

static inline double mathRandom_bubble()
{
	return Math::random();
}

EntityBubbleFX::EntityBubbleFX(World *world, double d, double d1, double d2,
                               double d3, double d4, double d5)
	: EntityFX(world, d, d1, d2, d3, d4, d5)
{
	particleRed = 1.0f;
	particleGreen = 1.0f;
	particleBlue = 1.0f;
	particleTextureIndex = 32;
	setSize(0.02f, 0.02f);
	particleScale = particleScale * (rand.nextFloat() * 0.6f + 0.2f);
	motionX = d3 * 0.20000000298023224 + (double)((float)(mathRandom_bubble() * 2.0 - 1.0) * 0.02f);
	motionY = d4 * 0.20000000298023224 + (double)((float)(mathRandom_bubble() * 2.0 - 1.0) * 0.02f);
	motionZ = d5 * 0.20000000298023224 + (double)((float)(mathRandom_bubble() * 2.0 - 1.0) * 0.02f);
	particleMaxAge = (int_t)(8.0 / (mathRandom_bubble() * 0.8 + 0.2));
}

void EntityBubbleFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	motionY += 0.002;
	moveEntity(motionX, motionY, motionZ);
	motionX *= 0.85000002384185791;
	motionY *= 0.85000002384185791;
	motionZ *= 0.85000002384185791;
	if (worldObj->getBlockMaterial(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ)) != Material::water)
	{
		setEntityDead();
	}
	if (particleMaxAge-- <= 0)
	{
		setEntityDead();
	}
}
