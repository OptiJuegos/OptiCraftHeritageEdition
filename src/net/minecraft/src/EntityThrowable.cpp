#include "EntityThrowable.h"
#include "java/Arithmetic.h"
#include "java/Math.h"

#include <cmath>

#include "AxisAlignedBB.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "MathHelper.h"
#include "MovingObjectPosition.h"
#include "NBTTagCompound.h"
#include "Vec3D.h"
#include "World.h"
#include "platform/PlatformTuning.h"

EntityThrowable::EntityThrowable(World *world)
	: Entity(world)
{
	ensureEntityInit();
	setSize(0.25f, 0.25f);
}

EntityThrowable::EntityThrowable(World *world, EntityLiving *thrower)
	: Entity(world)
{
	ensureEntityInit();
	throwerEntityId = thrower != nullptr ? thrower->entityId : -1;
	setSize(0.25f, 0.25f);
	if (thrower == nullptr)
		return;

	setLocationAndAngles(thrower->posX, thrower->posY + (double)thrower->getEyeHeight(), thrower->posZ, thrower->rotationYaw, thrower->rotationPitch);
	posX -= (double)(MathHelper::cos(rotationYaw / 180.0f * 3.1415927f) * 0.16f);
	posY -= static_cast<double>(0.1f);
	posZ -= (double)(MathHelper::sin(rotationYaw / 180.0f * 3.1415927f) * 0.16f);
	setPosition(posX, posY, posZ);
	yOffset = 0.0f;
	const float speed = 0.4f;
	motionX = (double)(-MathHelper::sin(rotationYaw / 180.0f * 3.1415927f) * MathHelper::cos(rotationPitch / 180.0f * 3.1415927f) * speed);
	motionZ = (double)(MathHelper::cos(rotationYaw / 180.0f * 3.1415927f) * MathHelper::cos(rotationPitch / 180.0f * 3.1415927f) * speed);
	motionY = (double)(-MathHelper::sin((rotationPitch + getInaccuracyPitchOffset()) / 180.0f * 3.1415927f) * speed);
	setThrowableHeading(motionX, motionY, motionZ, getVelocity(), 1.0f);
}

EntityThrowable::EntityThrowable(World *world, double x, double y, double z)
	: Entity(world)
{
	ensureEntityInit();
	setSize(0.25f, 0.25f);
	setPosition(x, y, z);
	yOffset = 0.0f;
}

void EntityThrowable::entityInit()
{
}

bool EntityThrowable::isInRangeToRenderDist(double distance)
{
	double range = boundingBox->getAverageEdgeLength() * 4.0;
	range *= 64.0;
	return distance < range * range;
}

EntityLiving *EntityThrowable::getThrower() const
{
	if (throwerEntityId < 0 || worldObj == nullptr)
		return nullptr;
	Entity *entity = worldObj->getEntityByID(throwerEntityId);
	return entity != nullptr && entity->isLiving() ? static_cast<EntityLiving *>(entity) : nullptr;
}

float EntityThrowable::getVelocity() const
{
	return 1.5f;
}

float EntityThrowable::getInaccuracyPitchOffset() const
{
	return 0.0f;
}

float EntityThrowable::getGravityVelocity() const
{
	return 0.03f;
}

void EntityThrowable::setThrowableHeading(double x, double y, double z, float velocity, float inaccuracy)
{
	float length = MathHelper::sqrt_double(x * x + y * y + z * z);
	if (length == 0.0f)
		return;
	x /= (double)length;
	y /= (double)length;
	z /= (double)length;
	x += rand.nextGaussian() * (double)0.0075f * (double)inaccuracy;
	y += rand.nextGaussian() * (double)0.0075f * (double)inaccuracy;
	z += rand.nextGaussian() * (double)0.0075f * (double)inaccuracy;
	x *= (double)velocity;
	y *= (double)velocity;
	z *= (double)velocity;
	motionX = x;
	motionY = y;
	motionZ = z;
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
	const float fx = static_cast<float>(x);
	const float fy = static_cast<float>(y);
	const float fz = static_cast<float>(z);
	const float horizontal = MathHelper::sqrt_float(fx * fx + fz * fz);
	prevRotationYaw = rotationYaw = std::atan2(fx, fz) * 180.0f / 3.1415927f;
	prevRotationPitch = rotationPitch = std::atan2(fy, horizontal) * 180.0f / 3.1415927f;
#else
	float horizontal = MathHelper::sqrt_double(x * x + z * z);
	prevRotationYaw = rotationYaw = (float)(JavaMath::atan2(x, z) * 180.0 / (double)3.1415927f);
	prevRotationPitch = rotationPitch = (float)(JavaMath::atan2(y, (double)horizontal) * 180.0 / (double)3.1415927f);
#endif
	ticksInGround = 0;
}

void EntityThrowable::setVelocity(double x, double y, double z)
{
	motionX = x;
	motionY = y;
	motionZ = z;
	if (prevRotationPitch == 0.0f && prevRotationYaw == 0.0f)
	{
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
		const float fx = static_cast<float>(x);
		const float fy = static_cast<float>(y);
		const float fz = static_cast<float>(z);
		const float horizontal = MathHelper::sqrt_float(fx * fx + fz * fz);
		prevRotationYaw = rotationYaw = std::atan2(fx, fz) * 180.0f / 3.1415927f;
		prevRotationPitch = rotationPitch = std::atan2(fy, horizontal) * 180.0f / 3.1415927f;
#else
		float horizontal = MathHelper::sqrt_double(x * x + z * z);
		prevRotationYaw = rotationYaw = (float)(JavaMath::atan2(x, z) * 180.0 / (double)3.1415927f);
		prevRotationPitch = rotationPitch = (float)(JavaMath::atan2(y, (double)horizontal) * 180.0 / (double)3.1415927f);
#endif
	}
}

void EntityThrowable::onUpdate()
{
	lastTickPosX = posX;
	lastTickPosY = posY;
	lastTickPosZ = posZ;
	Entity::onUpdate();

	if (throwableShake > 0)
		--throwableShake;

	if (inGround)
	{
		int_t blockId = worldObj->getBlockId(xTile, yTile, zTile);
		if (blockId == inTile)
		{
			++ticksInGround;
			if (ticksInGround == 1200)
				setEntityDead();
			return;
		}

		inGround = false;
		motionX *= (double)(rand.nextFloat() * 0.2f);
		motionY *= (double)(rand.nextFloat() * 0.2f);
		motionZ *= (double)(rand.nextFloat() * 0.2f);
		ticksInGround = 0;
		ticksInAir = 0;
	}
	else
	{
		++ticksInAir;
	}

	Vec3D *start = Vec3D::createVector(posX, posY, posZ);
	Vec3D *end = Vec3D::createVector(posX + motionX, posY + motionY, posZ + motionZ);
	MovingObjectPosition *blockHit = worldObj->rayTraceBlocks(start, end);
	start = Vec3D::createVector(posX, posY, posZ);
	end = Vec3D::createVector(posX + motionX, posY + motionY, posZ + motionZ);
	if (blockHit != nullptr)
		end = Vec3D::createVector(blockHit->hitVec->xCoord, blockHit->hitVec->yCoord, blockHit->hitVec->zCoord);

	Entity *entityHit = nullptr;
	if (!worldObj->multiplayerWorld)
	{
		const auto &entities = worldObj->getEntitiesWithinAABBExcludingEntity(this, boundingBox->addCoord(motionX, motionY, motionZ)->expand(1.0, 1.0, 1.0));
		double nearestDistance = 0.0;
		EntityLiving *thrower = getThrower();
		for (Entity *candidate : entities)
		{
			if (candidate == nullptr || !candidate->canBeCollidedWith() || (candidate == thrower && ticksInAir < 5))
				continue;
			AxisAlignedBB *box = candidate->boundingBox->expand(static_cast<double>(0.3f), static_cast<double>(0.3f), static_cast<double>(0.3f));
			MovingObjectPosition *intercept = box->calculateIntercept(start, end);
			if (intercept == nullptr)
				continue;
			double distance = start->distanceTo(intercept->hitVec);
			if (distance < nearestDistance || nearestDistance == 0.0)
			{
				entityHit = candidate;
				nearestDistance = distance;
			}
			delete intercept;
		}
	}

	if (entityHit != nullptr)
	{
		delete blockHit;
		blockHit = new MovingObjectPosition(entityHit);
	}

	if (blockHit != nullptr)
		onImpact(blockHit);
	delete blockHit;

	posX += motionX;
	posY += motionY;
	posZ += motionZ;
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
	const float motionXf = static_cast<float>(motionX);
	const float motionYf = static_cast<float>(motionY);
	const float motionZf = static_cast<float>(motionZ);
	const float horizontal = MathHelper::sqrt_float(motionXf * motionXf + motionZf * motionZf);
	rotationYaw = std::atan2(motionXf, motionZf) * 180.0f / 3.1415927f;
	rotationPitch = std::atan2(motionYf, horizontal) * 180.0f / 3.1415927f;
#else
	float horizontal = MathHelper::sqrt_double(motionX * motionX + motionZ * motionZ);
	rotationYaw = (float)(JavaMath::atan2(motionX, motionZ) * 180.0 / (double)3.1415927f);
	rotationPitch = (float)(JavaMath::atan2(motionY, (double)horizontal) * 180.0 / (double)3.1415927f);
#endif

	while (rotationPitch - prevRotationPitch < -180.0f)
		prevRotationPitch -= 360.0f;
	while (rotationPitch - prevRotationPitch >= 180.0f)
		prevRotationPitch += 360.0f;
	while (rotationYaw - prevRotationYaw < -180.0f)
		prevRotationYaw -= 360.0f;
	while (rotationYaw - prevRotationYaw >= 180.0f)
		prevRotationYaw += 360.0f;

	rotationPitch = prevRotationPitch + (rotationPitch - prevRotationPitch) * 0.2f;
	rotationYaw = prevRotationYaw + (rotationYaw - prevRotationYaw) * 0.2f;
	float drag = 0.99f;
	float gravity = getGravityVelocity();
	if (isInWater())
	{
		for (int_t i = 0; i < 4; ++i)
		{
			const float fraction = 0.25f;
			worldObj->spawnParticle("bubble", posX - motionX * (double)fraction, posY - motionY * (double)fraction, posZ - motionZ * (double)fraction, motionX, motionY, motionZ);
		}
		drag = 0.8f;
	}

	motionX *= (double)drag;
	motionY *= (double)drag;
	motionZ *= (double)drag;
	motionY -= (double)gravity;
	setPosition(posX, posY, posZ);
}

void EntityThrowable::writeEntityToNBT(NBTTagCompound *tag)
{
	tag->setShort("xTile", JavaArithmetic::shortFromBits(static_cast<ushort_t>(xTile)));
	tag->setShort("yTile", JavaArithmetic::shortFromBits(static_cast<ushort_t>(yTile)));
	tag->setShort("zTile", JavaArithmetic::shortFromBits(static_cast<ushort_t>(zTile)));
	tag->setByte("inTile", JavaArithmetic::byteFromBits(static_cast<ubyte_t>(inTile)));
	tag->setByte("shake", JavaArithmetic::byteFromBits(static_cast<ubyte_t>(throwableShake)));
	tag->setByte("inGround", (byte_t)(inGround ? 1 : 0));
}

void EntityThrowable::readEntityFromNBT(NBTTagCompound *tag)
{
	xTile = tag->getShort("xTile");
	yTile = tag->getShort("yTile");
	zTile = tag->getShort("zTile");
	inTile = tag->getByte("inTile") & 255;
	throwableShake = tag->getByte("shake") & 255;
	inGround = tag->getByte("inGround") == 1;
}

void EntityThrowable::onCollideWithPlayer(EntityPlayer *player)
{
	(void)player;
}

float EntityThrowable::getShadowSize()
{
	return 0.0f;
}
