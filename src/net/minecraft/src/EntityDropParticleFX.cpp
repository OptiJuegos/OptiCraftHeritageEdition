#include "EntityDropParticleFX.h"

#include "BlockFluid.h"
#include "Material.h"
#include "MathHelper.h"
#include "World.h"
#include "java/Math.h"

#include "java/Arithmetic.h"
EntityDropParticleFX::EntityDropParticleFX(World *world, double x, double y, double z, Material *material)
	: EntityFX(world, x, y, z, 0.0, 0.0, 0.0), materialType(material), bobTimer(40)
{
	motionX = motionY = motionZ = 0.0;
	if (materialType == Material::water)
	{
		particleRed = 0.0f;
		particleGreen = 0.0f;
		particleBlue = 1.0f;
	}
	else
	{
		particleRed = 1.0f;
		particleGreen = 0.0f;
		particleBlue = 0.0f;
	}
	setParticleTextureIndex(113);
	setSize(0.01f, 0.01f);
	particleGravity = 0.06f;
	particleMaxAge = JavaArithmetic::doubleToInt(64.0 / (Math::random() * 0.8 + 0.2));
}

int_t EntityDropParticleFX::getBrightnessForRender(float partialTick)
{
	return materialType == Material::water ? EntityFX::getBrightnessForRender(partialTick) : 257;
}

float EntityDropParticleFX::getEntityBrightness(float partialTick)
{
	return materialType == Material::water ? EntityFX::getEntityBrightness(partialTick) : 1.0f;
}

void EntityDropParticleFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;

	if (materialType == Material::water)
	{
		particleRed = 0.2f;
		particleGreen = 0.3f;
		particleBlue = 1.0f;
	}
	else
	{
		particleRed = 1.0f;
		particleGreen = 16.0f / static_cast<float>(40 - bobTimer + 16);
		particleBlue = 4.0f / static_cast<float>(40 - bobTimer + 8);
	}

	motionY -= static_cast<double>(particleGravity);
	if (bobTimer-- > 0)
	{
		motionX *= 0.02;
		motionY *= 0.02;
		motionZ *= 0.02;
		setParticleTextureIndex(113);
	}
	else
	{
		setParticleTextureIndex(112);
	}

	moveEntity(motionX, motionY, motionZ);
	motionX *= static_cast<double>(0.98f);
	motionY *= static_cast<double>(0.98f);
	motionZ *= static_cast<double>(0.98f);
	if (particleMaxAge-- <= 0)
		setEntityDead();

	if (onGround)
	{
		if (materialType == Material::water)
		{
			setEntityDead();
			worldObj->spawnParticle("splash", posX, posY, posZ, 0.0, 0.0, 0.0);
		}
		else
		{
			setParticleTextureIndex(114);
		}
		motionX *= static_cast<double>(0.7f);
		motionZ *= static_cast<double>(0.7f);
	}

	const int_t blockX = MathHelper::floor_double(posX);
	const int_t blockY = MathHelper::floor_double(posY);
	const int_t blockZ = MathHelper::floor_double(posZ);
	Material *material = worldObj->getBlockMaterial(blockX, blockY, blockZ);
	if (material != nullptr && (material->getIsLiquid() || material->isSolid()))
	{
		const float surfaceY = static_cast<float>(blockY + 1) -
		    BlockFluid::getFluidHeightPercent(worldObj->getBlockMetadata(blockX, blockY, blockZ));
		if (posY < static_cast<double>(surfaceY))
			setEntityDead();
	}
}
