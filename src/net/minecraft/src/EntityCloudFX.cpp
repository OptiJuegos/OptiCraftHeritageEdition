#include "EntityCloudFX.h"

#include "AxisAlignedBB.h"
#include "EntityPlayer.h"
#include "World.h"
#include "java/Math.h"

#include "java/Arithmetic.h"
EntityCloudFX::EntityCloudFX(World *world, double x, double y, double z,
                             double motionXIn, double motionYIn, double motionZIn)
	: EntityFX(world, x, y, z, 0.0, 0.0, 0.0), initialScale(0.0f)
{
	const float sizeMultiplier = 2.5f;
	motionX *= static_cast<double>(0.1f);
	motionY *= static_cast<double>(0.1f);
	motionZ *= static_cast<double>(0.1f);
	motionX += motionXIn;
	motionY += motionYIn;
	motionZ += motionZIn;
	const float randomShade = static_cast<float>(Math::random());
	particleRed = particleGreen = particleBlue = 1.0f - randomShade * 0.3f;
	particleScale *= 12.0f / 16.0f;
	particleScale *= sizeMultiplier;
	initialScale = particleScale;
	particleMaxAge = JavaArithmetic::doubleToInt(8.0 / (Math::random() * 0.8 + 0.3));
	particleMaxAge = JavaArithmetic::floatToInt(static_cast<float>(particleMaxAge) * sizeMultiplier);
	noClip = false;
}

void EntityCloudFX::renderParticle(Tessellator *tessellator, float partialTick,
                                   float rotationX, float rotationXZ, float rotationZ,
                                   float rotationYZ, float rotationXY)
{
	float ageScale = (static_cast<float>(particleAge) + partialTick) /
	                 static_cast<float>(particleMaxAge) * 32.0f;
	if (ageScale < 0.0f)
		ageScale = 0.0f;
	if (ageScale > 1.0f)
		ageScale = 1.0f;
	particleScale = initialScale * ageScale;
	EntityFX::renderParticle(tessellator, partialTick, rotationX, rotationXZ,
	                         rotationZ, rotationYZ, rotationXY);
}

void EntityCloudFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	if (particleAge++ >= particleMaxAge)
		setEntityDead();

	setParticleTextureIndex(7 - particleAge * 8 / particleMaxAge);
	moveEntity(motionX, motionY, motionZ);
	motionX *= static_cast<double>(0.96f);
	motionY *= static_cast<double>(0.96f);
	motionZ *= static_cast<double>(0.96f);

	EntityPlayer *player = worldObj->getClosestPlayerToEntity(this, 2.0);
	if (player != nullptr && player->boundingBox != nullptr && posY > player->boundingBox->minY)
	{
		posY += (player->boundingBox->minY - posY) * 0.2;
		motionY += (player->motionY - motionY) * 0.2;
		setPosition(posX, posY, posZ);
	}

	if (onGround)
	{
		motionX *= static_cast<double>(0.7f);
		motionZ *= static_cast<double>(0.7f);
	}
}
