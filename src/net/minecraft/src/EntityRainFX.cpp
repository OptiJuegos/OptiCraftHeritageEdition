#include "EntityRainFX.h"
#include "java/Math.h"

#include <cstdlib>

#include "BlockFluid.h"
#include "MathHelper.h"
#include "Material.h"
#include "Tessellator.h"
#include "World.h"

static inline double mathRandom_rain()
{
	return Math::random();
}

EntityRainFX::EntityRainFX(World *world, double d, double d1, double d2)
	: EntityFX(world, d, d1, d2, 0.0, 0.0, 0.0)
{
	motionX *= 0.30000001192092896;
	motionY = (float)mathRandom_rain() * 0.2f + 0.1f;
	motionZ *= 0.30000001192092896;
	particleRed = 1.0f;
	particleGreen = 1.0f;
	particleBlue = 1.0f;
	particleTextureIndex = 19 + rand.nextInt(4);
	setSize(0.01f, 0.01f);
	particleGravity = 0.06f;
	particleMaxAge = (int_t)(8.0 / (mathRandom_rain() * 0.8 + 0.2));
}

void EntityRainFX::renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5)
{
	EntityFX::renderParticle(tessellator, f, f1, f2, f3, f4, f5);
}

void EntityRainFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	motionY -= particleGravity;
	moveEntity(motionX, motionY, motionZ);
	motionX *= 0.98000001907348633;
	motionY *= 0.98000001907348633;
	motionZ *= 0.98000001907348633;
	if (particleMaxAge-- <= 0)
	{
		setEntityDead();
	}
	if (onGround)
	{
		if (mathRandom_rain() < 0.5)
		{
			setEntityDead();
		}
		motionX *= 0.69999998807907104;
		motionZ *= 0.69999998807907104;
	}
	Material *material = worldObj->getBlockMaterial(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ));
	if (material->getIsLiquid() || material->isSolid())
	{
		double d = (double)(float)(MathHelper::floor_double(posY) + 1) - BlockFluid::getPercentAir(worldObj->getBlockMetadata(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ)));
		if (posY < d)
		{
			setEntityDead();
		}
	}
}
