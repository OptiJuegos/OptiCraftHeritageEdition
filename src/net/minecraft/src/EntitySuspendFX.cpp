#include "EntitySuspendFX.h"

#include "Material.h"
#include "MathHelper.h"
#include "World.h"
#include "java/Math.h"

#include "java/Arithmetic.h"
EntitySuspendFX::EntitySuspendFX(World *world, double x, double y, double z,
                                 double motionXIn, double motionYIn, double motionZIn)
	: EntityFX(world, x, y - 0.125, z, motionXIn, motionYIn, motionZIn)
{
	particleRed = 0.4f;
	particleGreen = 0.4f;
	particleBlue = 0.7f;
	setParticleTextureIndex(0);
	setSize(0.01f, 0.01f);
	particleScale *= rand.nextFloat() * 0.6f + 0.2f;
	motionX = 0.0;
	motionY = 0.0;
	motionZ = 0.0;
	particleMaxAge = JavaArithmetic::doubleToInt(16.0 / (Math::random() * 0.8 + 0.2));
}

void EntitySuspendFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	moveEntity(motionX, motionY, motionZ);
	if (worldObj->getBlockMaterial(MathHelper::floor_double(posX), MathHelper::floor_double(posY),
	                               MathHelper::floor_double(posZ)) != Material::water)
		setEntityDead();
	if (particleMaxAge-- <= 0)
		setEntityDead();
}
