#include "EntityEnchantmentTableParticleFX.h"

#include "java/Math.h"

#include "java/Arithmetic.h"
EntityEnchantmentTableParticleFX::EntityEnchantmentTableParticleFX(
    World *world, double x, double y, double z,
    double motionXIn, double motionYIn, double motionZIn)
	: EntityFX(world, x, y, z, motionXIn, motionYIn, motionZIn),
	  initialScale(0.0f), originX(x), originY(y), originZ(z)
{
	motionX = motionXIn;
	motionY = motionYIn;
	motionZ = motionZIn;
	posX = originX;
	posY = originY;
	posZ = originZ;
	const float shade = rand.nextFloat() * 0.6f + 0.4f;
	initialScale = particleScale = rand.nextFloat() * 0.5f + 0.2f;
	particleRed = particleGreen = particleBlue = shade;
	particleGreen *= 0.9f;
	particleRed *= 0.9f;
	particleMaxAge = JavaArithmetic::doubleToInt(Math::random() * 10.0) + 30;
	noClip = true;
	setParticleTextureIndex(JavaArithmetic::doubleToInt(Math::random() * 26.0 + 225.0));
}

int_t EntityEnchantmentTableParticleFX::getBrightnessForRender(float partialTick)
{
	const int_t packed = EntityFX::getBrightnessForRender(partialTick);
	float age = static_cast<float>(particleAge) / static_cast<float>(particleMaxAge);
	age *= age;
	age *= age;
	const int_t low = packed & 255;
	int_t high = packed >> 16 & 255;
	high += static_cast<int_t>(age * 15.0f * 16.0f);
	if (high > 240)
		high = 240;
	return low | high << 16;
}

float EntityEnchantmentTableParticleFX::getEntityBrightness(float partialTick)
{
	const float base = EntityFX::getEntityBrightness(partialTick);
	float age = static_cast<float>(particleAge) / static_cast<float>(particleMaxAge);
	age *= age;
	age *= age;
	return base * (1.0f - age) + age;
}

void EntityEnchantmentTableParticleFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	float remaining = static_cast<float>(particleAge) / static_cast<float>(particleMaxAge);
	remaining = 1.0f - remaining;
	float curve = 1.0f - remaining;
	curve *= curve;
	curve *= curve;
	posX = originX + motionX * static_cast<double>(remaining);
	posY = originY + motionY * static_cast<double>(remaining) - static_cast<double>(curve * 1.2f);
	posZ = originZ + motionZ * static_cast<double>(remaining);
	if (particleAge++ >= particleMaxAge)
		setEntityDead();
}
