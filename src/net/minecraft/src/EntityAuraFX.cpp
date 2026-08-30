#include "EntityAuraFX.h"

#include "java/Math.h"

#include "java/Arithmetic.h"
EntityAuraFX::EntityAuraFX(World *world, double x, double y, double z,
                           double motionXIn, double motionYIn, double motionZIn)
	: EntityFX(world, x, y, z, motionXIn, motionYIn, motionZIn)
{
	const float shade = rand.nextFloat() * 0.1f + 0.2f;
	particleRed = shade;
	particleGreen = shade;
	particleBlue = shade;
	setParticleTextureIndex(0);
	setSize(0.02f, 0.02f);
	particleScale *= rand.nextFloat() * 0.6f + 0.5f;
	motionX *= static_cast<double>(0.02f);
	motionY *= static_cast<double>(0.02f);
	motionZ *= static_cast<double>(0.02f);
	particleMaxAge = JavaArithmetic::doubleToInt(20.0 / (Math::random() * 0.8 + 0.2));
	noClip = true;
}

void EntityAuraFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	moveEntity(motionX, motionY, motionZ);
	motionX *= 0.99;
	motionY *= 0.99;
	motionZ *= 0.99;
	if (particleMaxAge-- <= 0)
		setEntityDead();
}
