#include "EntityFireball.h"
#include "java/Arithmetic.h"
#include "java/Math.h"

#include <cmath>

#include "AxisAlignedBB.h"
#include "DamageSource.h"
#include "EntityLiving.h"
#include "MathHelper.h"
#include "MovingObjectPosition.h"
#include "NBTTagCompound.h"
#include "Vec3D.h"
#include "World.h"
#include "Explosion.h"
#include "platform/PlatformTuning.h"

EntityFireball::EntityFireball(World *world)
	: Entity(world), shootingEntity(nullptr), accelerationX(0.0), accelerationY(0.0), accelerationZ(0.0),
	  xTile(-1), yTile(-1), zTile(-1), inTile(0), inGround(false), ticksAlive(0), ticksInAir(0), shootingEntityId(-1)
{
	setSize(1.0f, 1.0f);
}

EntityFireball::EntityFireball(World *world, double x, double y, double z,
	double xAcceleration, double yAcceleration, double zAcceleration)
	: EntityFireball(world)
{
	setLocationAndAngles(x, y, z, rotationYaw, rotationPitch);
	setPosition(x, y, z);
	const double length = (double)MathHelper::sqrt_double(
		xAcceleration * xAcceleration + yAcceleration * yAcceleration + zAcceleration * zAcceleration);
	if (length > 0.0)
	{
		accelerationX = xAcceleration / length * 0.1;
		accelerationY = yAcceleration / length * 0.1;
		accelerationZ = zAcceleration / length * 0.1;
	}
}

EntityFireball::EntityFireball(World *world, EntityLiving *shooter,
	double xAcceleration, double yAcceleration, double zAcceleration)
	: EntityFireball(world)
{
	if (shooter == nullptr)
		return;
	setShootingEntity(shooter);
	setLocationAndAngles(shooter->posX, shooter->posY, shooter->posZ, shooter->rotationYaw, shooter->rotationPitch);
	setPosition(posX, posY, posZ);
	yOffset = 0.0f;
	motionX = motionY = motionZ = 0.0;
	xAcceleration += rand.nextGaussian() * 0.4;
	yAcceleration += rand.nextGaussian() * 0.4;
	zAcceleration += rand.nextGaussian() * 0.4;
	const double length = (double)MathHelper::sqrt_double(
		xAcceleration * xAcceleration + yAcceleration * yAcceleration + zAcceleration * zAcceleration);
	if (length > 0.0)
	{
		accelerationX = xAcceleration / length * 0.1;
		accelerationY = yAcceleration / length * 0.1;
		accelerationZ = zAcceleration / length * 0.1;
	}
}

void EntityFireball::entityInit()
{
}

void EntityFireball::setShootingEntity(EntityLiving *shooter)
{
	shootingEntity = shooter;
	shootingEntityId = shooter != nullptr ? shooter->entityId : -1;
}

EntityLiving *EntityFireball::getShootingEntity()
{
	if (worldObj == nullptr || shootingEntityId < 0)
	{
		shootingEntity = nullptr;
		return nullptr;
	}
	Entity *entity = worldObj->getEntityByID(shootingEntityId);
	shootingEntity = entity != nullptr && entity->isLiving() ? static_cast<EntityLiving *>(entity) : nullptr;
	return shootingEntity;
}

bool EntityFireball::isInRangeToRenderDist(double distance)
{
	double range = boundingBox->getAverageEdgeLength() * 4.0;
	range *= 64.0;
	return distance < range * range;
}

void EntityFireball::onUpdate()
{
	EntityLiving *shooter = getShootingEntity();
	if (!worldObj->multiplayerWorld && ((shootingEntityId >= 0 && (shooter == nullptr || shooter->isDead)) ||
		!worldObj->blockExists(JavaArithmetic::doubleToInt(posX), JavaArithmetic::doubleToInt(posY), JavaArithmetic::doubleToInt(posZ))))
	{
		setEntityDead();
		return;
	}

	Entity::onUpdate();
	setFire(1);

	if (inGround)
	{
		const int_t blockId = worldObj->getBlockId(xTile, yTile, zTile);
		if (blockId == inTile)
		{
			++ticksAlive;
			if (ticksAlive == 600)
				setEntityDead();
			return;
		}
		inGround = false;
		motionX *= (double)(rand.nextFloat() * 0.2f);
		motionY *= (double)(rand.nextFloat() * 0.2f);
		motionZ *= (double)(rand.nextFloat() * 0.2f);
		ticksAlive = 0;
		ticksInAir = 0;
	}
	else
	{
		++ticksInAir;
	}

	Vec3D *start = Vec3D::createVector(posX, posY, posZ);
	Vec3D *end = Vec3D::createVector(posX + motionX, posY + motionY, posZ + motionZ);
	MovingObjectPosition *hit = worldObj->rayTraceBlocks(start, end);
	start = Vec3D::createVector(posX, posY, posZ);
	end = Vec3D::createVector(posX + motionX, posY + motionY, posZ + motionZ);
	if (hit != nullptr)
		end = Vec3D::createVector(hit->hitVec->xCoord, hit->hitVec->yCoord, hit->hitVec->zCoord);

	Entity *closestEntity = nullptr;
	double closestDistance = 0.0;
	const std::vector<Entity *> &entities = worldObj->getEntitiesWithinAABBExcludingEntity(
		this, boundingBox->addCoord(motionX, motionY, motionZ)->expand(1.0, 1.0, 1.0));
	for (Entity *candidate : entities)
	{
		if (candidate == nullptr || !candidate->canBeCollidedWith() || (candidate == shooter && ticksInAir < 25))
			continue;
		AxisAlignedBB *box = candidate->boundingBox->expand(static_cast<double>(0.3f), static_cast<double>(0.3f), static_cast<double>(0.3f));
		MovingObjectPosition *candidateHit = box->calculateIntercept(start, end);
		if (candidateHit == nullptr)
			continue;
		const double distance = start->distanceTo(candidateHit->hitVec);
		if (distance < closestDistance || closestDistance == 0.0)
		{
			closestEntity = candidate;
			closestDistance = distance;
		}
		delete candidateHit;
	}

	if (closestEntity != nullptr)
	{
		delete hit;
		hit = new MovingObjectPosition(closestEntity);
	}

	if (hit != nullptr)
		onImpact(hit);
	delete hit;

	posX += motionX;
	posY += motionY;
	posZ += motionZ;
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
	const float motionXf = static_cast<float>(motionX);
	const float motionYf = static_cast<float>(motionY);
	const float motionZf = static_cast<float>(motionZ);
	const float horizontal = MathHelper::sqrt_float(motionXf * motionXf + motionZf * motionZf);
	rotationYaw = std::atan2(motionXf, motionZf) * 180.0f / 3.1415927f;
	for (rotationPitch = std::atan2(motionYf, horizontal) * 180.0f / 3.1415927f;
#else
	const float horizontal = MathHelper::sqrt_double(motionX * motionX + motionZ * motionZ);
	rotationYaw = (float)(JavaMath::atan2(motionX, motionZ) * 180.0 / 3.1415927410125732);
	for (rotationPitch = (float)(JavaMath::atan2(motionY, (double)horizontal) * 180.0 / 3.1415927410125732);
#endif
		rotationPitch - prevRotationPitch < -180.0f; prevRotationPitch -= 360.0f) {}
	while (rotationPitch - prevRotationPitch >= 180.0f) prevRotationPitch += 360.0f;
	while (rotationYaw - prevRotationYaw < -180.0f) prevRotationYaw -= 360.0f;
	while (rotationYaw - prevRotationYaw >= 180.0f) prevRotationYaw += 360.0f;
	rotationPitch = prevRotationPitch + (rotationPitch - prevRotationPitch) * 0.2f;
	rotationYaw = prevRotationYaw + (rotationYaw - prevRotationYaw) * 0.2f;

	float drag = 0.95f;
	if (isInWater())
	{
		for (int_t i = 0; i < 4; ++i)
			worldObj->spawnParticle("bubble", posX - motionX * 0.25, posY - motionY * 0.25, posZ - motionZ * 0.25,
				motionX, motionY, motionZ);
		drag = 0.8f;
	}
	motionX += accelerationX;
	motionY += accelerationY;
	motionZ += accelerationZ;
	motionX *= (double)drag;
	motionY *= (double)drag;
	motionZ *= (double)drag;
	worldObj->spawnParticle("smoke", posX, posY + 0.5, posZ, 0.0, 0.0, 0.0);
	setPosition(posX, posY, posZ);
}

void EntityFireball::onImpact(MovingObjectPosition *hit)
{
	if (worldObj->multiplayerWorld || hit == nullptr)
		return;
	EntityLiving *shooter = getShootingEntity();
	if (hit->entityHit != nullptr)
	{
		DamageSource source = DamageSource::causeFireballDamage(this, shooter);
		hit->entityHit->attackEntityFrom(source, 4);
	}
	delete worldObj->newExplosion(nullptr, posX, posY, posZ, 1.0f, true);
	setEntityDead();
}

void EntityFireball::writeEntityToNBT(NBTTagCompound *compound)
{
	compound->setShort("xTile", JavaArithmetic::shortFromBits(static_cast<ushort_t>(xTile)));
	compound->setShort("yTile", JavaArithmetic::shortFromBits(static_cast<ushort_t>(yTile)));
	compound->setShort("zTile", JavaArithmetic::shortFromBits(static_cast<ushort_t>(zTile)));
	compound->setByte("inTile", JavaArithmetic::byteFromBits(static_cast<ubyte_t>(inTile)));
	compound->setByte("inGround", (byte_t)(inGround ? 1 : 0));
}

void EntityFireball::readEntityFromNBT(NBTTagCompound *compound)
{
	xTile = compound->getShort("xTile");
	yTile = compound->getShort("yTile");
	zTile = compound->getShort("zTile");
	inTile = compound->getByte("inTile") & 0xff;
	inGround = compound->getByte("inGround") == 1;
}

bool EntityFireball::canBeCollidedWith()
{
	return true;
}

float EntityFireball::getCollisionBorderSize()
{
	return 1.0f;
}

bool EntityFireball::attackEntityFrom(Entity *entity, int_t)
{
	setBeenAttacked();
	if (entity == nullptr)
		return false;
	Vec3D *look = entity->getLookVec();
	if (look != nullptr)
	{
		motionX = look->xCoord;
		motionY = look->yCoord;
		motionZ = look->zCoord;
		accelerationX = motionX * 0.1;
		accelerationY = motionY * 0.1;
		accelerationZ = motionZ * 0.1;
	}
	if (entity->isLiving())
		setShootingEntity(static_cast<EntityLiving *>(entity));
	return true;
}

bool EntityFireball::attackEntityFrom(const DamageSource &source, int_t damage)
{
	return attackEntityFrom(source.getEntity(), damage);
}

float EntityFireball::getShadowSize()
{
	return 0.0f;
}

float EntityFireball::getBrightness(float)
{
	return 1.0f;
}

int_t EntityFireball::getBrightnessForRender(float)
{
	return 15728880;
}
