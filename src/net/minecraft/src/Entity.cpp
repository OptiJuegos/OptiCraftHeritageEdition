#include "Entity.h"
#include "java/Arithmetic.h"

#include <cmath>
#include <string>
#include <vector>

#include "AxisAlignedBB.h"
#include "Block.h"
#include "BlockFluid.h"
#include "DataWatcher.h"
#include "DamageSource.h"
#include "EntityItem.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "EntityMob.h"
#include "EntityAnimal.h"
#include "EntityWaterMob.h"
#include "EntityList.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "Material.h"
#include "NBTTagCompound.h"
#include "NBTTagDouble.h"
#include "NBTTagFloat.h"
#include "NBTTagList.h"
#include "platform/PlatformTuning.h"
#include "StepSound.h"
#include "Vec3D.h"
#include "World.h"

int_t Entity::nextEntityID = 0;

namespace
{
	inline bool hasWorldCollision(World *world, Entity *entity, AxisAlignedBB *bounds)
	{
#if PLATFORM_EARLY_COLLISION_EXIT
		return world->hasCollidingBoundingBoxes(entity, bounds);
#else
		return !world->getCollidingBoundingBoxes(entity, bounds).empty();
#endif
	}

#if PLATFORM_FLOAT_COLLISION_SWEEP
	// Float mirror of the entity's bounding box in the sweep's local frame.
	struct SweepLocalBox
	{
		float minX, minY, minZ, maxX, maxY, maxZ;
	};

	// Reused across moves so the candidate list never reallocates in steady
	// state; moveEntity is not re-entered while a sweep is live.
	PlatformCollisionSweep s_collisionSweep;

	inline void sweepRebase(const AxisAlignedBB *bounds, const PlatformCollisionSweep &sweep, SweepLocalBox &out)
	{
		out.minX = (float)(bounds->minX - (double)sweep.originX);
		out.minY = (float)(bounds->minY - (double)sweep.originY);
		out.minZ = (float)(bounds->minZ - (double)sweep.originZ);
		out.maxX = (float)(bounds->maxX - (double)sweep.originX);
		out.maxY = (float)(bounds->maxY - (double)sweep.originY);
		out.maxZ = (float)(bounds->maxZ - (double)sweep.originZ);
	}

	// AxisAlignedBB::calculate{X,Y,Z}Offset over every candidate, in float. When
	// a box shortens the move, `face` receives the local coordinate of the face
	// the entity ends up touching and true is returned; otherwise the requested
	// distance stands untouched, which is what the caller's `d6 != d1` tests
	// depend on.
	inline bool sweepAxisX(const PlatformCollisionSweep &sweep, const SweepLocalBox &e, float d, float &face)
	{
		bool clipped = false;
		for (const PlatformCollisionSweepBox &b : sweep.boxes)
		{
			if (e.maxY <= b.minY || e.minY >= b.maxY) continue;
			if (e.maxZ <= b.minZ || e.minZ >= b.maxZ) continue;
			if (d > 0.0f && e.maxX <= b.minX)
			{
				const float gap = b.minX - e.maxX;
				if (gap < d) { d = gap; face = b.minX; clipped = true; }
			}
			if (d < 0.0f && e.minX >= b.maxX)
			{
				const float gap = b.maxX - e.minX;
				if (gap > d) { d = gap; face = b.maxX; clipped = true; }
			}
		}
		return clipped;
	}

	inline bool sweepAxisY(const PlatformCollisionSweep &sweep, const SweepLocalBox &e, float d, float &face)
	{
		bool clipped = false;
		for (const PlatformCollisionSweepBox &b : sweep.boxes)
		{
			if (e.maxX <= b.minX || e.minX >= b.maxX) continue;
			if (e.maxZ <= b.minZ || e.minZ >= b.maxZ) continue;
			if (d > 0.0f && e.maxY <= b.minY)
			{
				const float gap = b.minY - e.maxY;
				if (gap < d) { d = gap; face = b.minY; clipped = true; }
			}
			if (d < 0.0f && e.minY >= b.maxY)
			{
				const float gap = b.maxY - e.minY;
				if (gap > d) { d = gap; face = b.maxY; clipped = true; }
			}
		}
		return clipped;
	}

	inline bool sweepAxisZ(const PlatformCollisionSweep &sweep, const SweepLocalBox &e, float d, float &face)
	{
		bool clipped = false;
		for (const PlatformCollisionSweepBox &b : sweep.boxes)
		{
			if (e.maxX <= b.minX || e.minX >= b.maxX) continue;
			if (e.maxY <= b.minY || e.minY >= b.maxY) continue;
			if (d > 0.0f && e.maxZ <= b.minZ)
			{
				const float gap = b.minZ - e.maxZ;
				if (gap < d) { d = gap; face = b.minZ; clipped = true; }
			}
			if (d < 0.0f && e.minZ >= b.maxZ)
			{
				const float gap = b.maxZ - e.minZ;
				if (gap > d) { d = gap; face = b.maxZ; clipped = true; }
			}
		}
		return clipped;
	}

	// Turns a clipped float result back into an exact double delta: the face is
	// an integer-plus-fraction block coordinate, so origin + face is exact, and
	// the subtraction against the entity's own coordinate is exact by Sterbenz
	// -- which is what makes vanilla land entities ON the block instead of a
	// float approximation of it. Never moves further than was asked.
	inline double sweepExactDelta(double requested, int_t origin, float face, double minEdge, double maxEdge)
	{
		const double target = (double)origin + (double)face;
		if (requested > 0.0)
		{
			const double delta = target - maxEdge;
			return delta < requested ? delta : requested;
		}
		const double delta = target - minEdge;
		return delta > requested ? delta : requested;
	}

	// One axis of the sweep, applied to the double box. Only the two edges on
	// that axis move (AxisAlignedBB::offset adds all six).
	inline void sweepApplyX(const PlatformCollisionSweep &sweep, AxisAlignedBB *bounds, SweepLocalBox &e, double &d)
	{
		if (d == 0.0)
			return;
		float face = 0.0f;
		const bool clipped = sweepAxisX(sweep, e, (float)d, face);
		if (clipped)
			d = sweepExactDelta(d, sweep.originX, face, bounds->minX, bounds->maxX);
		bounds->minX += d;
		bounds->maxX += d;
		if (clipped)
		{
			e.minX = (float)(bounds->minX - (double)sweep.originX);
			e.maxX = (float)(bounds->maxX - (double)sweep.originX);
		}
		else
		{
			e.minX += (float)d;
			e.maxX += (float)d;
		}
	}

	inline void sweepApplyY(const PlatformCollisionSweep &sweep, AxisAlignedBB *bounds, SweepLocalBox &e, double &d1)
	{
		if (d1 == 0.0)
			return;
		float face = 0.0f;
		const bool clipped = sweepAxisY(sweep, e, (float)d1, face);
		if (clipped)
			d1 = sweepExactDelta(d1, sweep.originY, face, bounds->minY, bounds->maxY);
		bounds->minY += d1;
		bounds->maxY += d1;
		if (clipped)
		{
			e.minY = (float)(bounds->minY - (double)sweep.originY);
			e.maxY = (float)(bounds->maxY - (double)sweep.originY);
		}
		else
		{
			e.minY += (float)d1;
			e.maxY += (float)d1;
		}
	}

	inline void sweepApplyZ(const PlatformCollisionSweep &sweep, AxisAlignedBB *bounds, SweepLocalBox &e, double &d2)
	{
		if (d2 == 0.0)
			return;
		float face = 0.0f;
		const bool clipped = sweepAxisZ(sweep, e, (float)d2, face);
		if (clipped)
			d2 = sweepExactDelta(d2, sweep.originZ, face, bounds->minZ, bounds->maxZ);
		bounds->minZ += d2;
		bounds->maxZ += d2;
		if (clipped)
		{
			e.minZ = (float)(bounds->minZ - (double)sweep.originZ);
			e.maxZ = (float)(bounds->maxZ - (double)sweep.originZ);
		}
		else
		{
			e.minZ += (float)d2;
			e.maxZ += (float)d2;
		}
	}
#endif // PLATFORM_FLOAT_COLLISION_SWEEP
}

bool Entity::isAssignableTo(const std::type_info &type) const
{
	if (type == typeid(Entity))
		return true;
	if (type == typeid(EntityLiving))
		return isLiving();
	if (type == typeid(EntityPlayer))
		return isPlayer();
	if (type == typeid(EntityMob))
		return isMob();
	if (type == typeid(EntityAnimal))
		return isAnimal();
	if (type == typeid(EntityWaterMob))
		return isWaterMob();
	return typeid(*this) == type;
}

Entity::Entity(World *world) :
	boundingBoxStorage(),
	boundingBox(&boundingBoxStorage)
{
	entityId = nextEntityID;
	nextEntityID = JavaArithmetic::intAdd(nextEntityID, 1);
	renderDistanceWeight = 1.0;
	preventEntitySpawning = false;
	riddenByEntity = nullptr;
	ridingEntity = nullptr;
	prevPosX = prevPosY = prevPosZ = 0.0;
	posX = posY = posZ = 0.0;
	motionX = motionY = motionZ = 0.0;
	rotationYaw = rotationPitch = 0.0f;
	prevRotationYaw = prevRotationPitch = 0.0f;
	onGround = false;
	isCollidedHorizontally = false;
	isCollidedVertically = false;
	isCollided = false;
	isAirBorne = false;
	beenAttacked = false;
	velocityChanged = false;
	isInWeb = false;
	field_9293_aM = true;
	isDead = false;
	yOffset = 0.0f;
	width = 0.6f;
	height = 1.8f;
	prevDistanceWalkedModified = 0.0f;
	distanceWalkedModified = 0.0f;
	fallDistance = 0.0f;
	nextStepDistance = 1;
	lastTickPosX = lastTickPosY = lastTickPosZ = 0.0;
	ySize = 0.0f;
	stepHeight = 0.0f;
	noClip = false;
	entityCollisionReduction = 0.0f;
	ticksExisted = 0;
	fireResistance = 1;
	fire = 0;
	maxAir = 300;
	inWater = false;
	heartsLife = 0;
	air = 300;
	isFirstUpdate = true;
	immuneToFire = false;
	dataWatcher = new DataWatcher();
	entityBrightness = 0.0f;
	entityRiderPitchDelta = 0.0;
	entityRiderYawDelta = 0.0;
	addedToChunk = false;
	chunkCoordX = chunkCoordY = chunkCoordZ = 0;
	serverPosX = serverPosY = serverPosZ = 0;
	ignoreFrustumCheck = false;
#if PLATFORM_CACHE_ENTITY_CHUNK_EXISTENCE
	chunkExistenceCacheMinChunkX = 0;
	chunkExistenceCacheMinChunkZ = 0;
	chunkExistenceCacheMaxChunkX = 0;
	chunkExistenceCacheMaxChunkZ = 0;
	chunkExistenceCacheTopologyVersion = 0;
	chunkExistenceCacheResult = false;
	chunkExistenceCacheValid = false;
#endif
	worldObj = world;
	setPosition(0.0, 0.0, 0.0);
	dataWatcher->addObject(0, (byte_t)0);
	dataWatcher->addObject(1, (short_t)300);
	// NOTE: entityInit() is called by subclass constructor since it's pure virtual.
}

Entity::~Entity()
{
	delete dataWatcher;
	// boundingBox now points at boundingBoxStorage (owned inline) — nothing to free.
}

DataWatcher *Entity::getDataWatcher()
{
	return dataWatcher;
}

bool Entity::equals(Entity *obj)
{
	if (obj != nullptr)
	{
		return obj->entityId == entityId;
	}
	return false;
}

bool Entity::isEntityEqual(Entity *entity)
{
	return this == entity;
}

int_t Entity::hashCode()
{
	return entityId;
}

void Entity::preparePlayerToSpawn()
{
	if (worldObj == nullptr)
	{
		return;
	}
	do
	{
		if (posY <= 0.0)
		{
			break;
		}
		setPosition(posX, posY, posZ);
		if (!hasWorldCollision(worldObj, this, boundingBox))
		{
			break;
		}
		posY++;
	} while (true);
	motionX = motionY = motionZ = 0.0;
	rotationPitch = 0.0f;
#ifdef PS2_PLATFORM
	// The spawn collision resolver above can move posY through several blocks
	// using setPosition(), which deliberately does not update interpolation
	// history. At desktop frame rates the stale prev/last position is barely
	// visible; on the PS2 it can persist for an entire slow world frame and
	// looks like the player is spawned in one place then falls/teleports.
	prevPosX = lastTickPosX = posX;
	prevPosY = lastTickPosY = posY;
	prevPosZ = lastTickPosZ = posZ;
	prevRotationYaw = rotationYaw;
	prevRotationPitch = rotationPitch;
#endif
}

void Entity::setEntityDead()
{
	isDead = true;
}

void Entity::setSize(float f, float f1)
{
	width = f;
	height = f1;
}

void Entity::setRotation(float f, float f1)
{
	rotationYaw = std::fmod(f, 360.0f);
	rotationPitch = std::fmod(f1, 360.0f);
}

void Entity::setPosition(double d, double d1, double d2)
{
	posX = d;
	posY = d1;
	posZ = d2;
	float f = width / 2.0f;
	float f1 = height;
	boundingBox->setBounds(d - (double)f, (d1 - (double)yOffset) + (double)ySize, d2 - (double)f,
	                       d + (double)f, (d1 - (double)yOffset) + (double)ySize + (double)f1, d2 + (double)f);
}

void Entity::setAngles(float f, float f1)
{
	float f2 = rotationPitch;
	float f3 = rotationYaw;
	rotationYaw += (float)((double)f * 0.15);
	rotationPitch -= (float)((double)f1 * 0.15);
	if (rotationPitch < -90.0f)
	{
		rotationPitch = -90.0f;
	}
	if (rotationPitch > 90.0f)
	{
		rotationPitch = 90.0f;
	}
	prevRotationPitch += rotationPitch - f2;
	prevRotationYaw += rotationYaw - f3;
}

void Entity::onUpdate()
{
	onEntityUpdate();
}

void Entity::onEntityUpdate()
{
	if (ridingEntity != nullptr && ridingEntity->isDead)
	{
		ridingEntity = nullptr;
	}
	ticksExisted++;
	prevDistanceWalkedModified = distanceWalkedModified;
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	prevRotationPitch = rotationPitch;
	prevRotationYaw = rotationYaw;
	if (isSprinting() && !isInWater())
	{
		const int_t blockX = MathHelper::floor_double(posX);
		const int_t blockY = MathHelper::floor_double(posY - static_cast<double>(0.2f) - (double)yOffset);
		const int_t blockZ = MathHelper::floor_double(posZ);
		const int_t blockId = worldObj->getBlockId(blockX, blockY, blockZ);
		if (blockId > 0)
		{
			const double particleX = posX + ((double)rand.nextFloat() - 0.5) * (double)width;
			const double particleZ = posZ + ((double)rand.nextFloat() - 0.5) * (double)width;
			worldObj->spawnParticle("tilecrack_" + std::to_string(blockId), particleX, boundingBox->minY + 0.1, particleZ,
			                       -motionX * 4.0, 1.5, -motionZ * 4.0);
		}
	}
	if (handleWaterMovement())
	{
		if (!inWater && !isFirstUpdate)
		{
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
			const float motionXf = (float)motionX;
			const float motionYf = (float)motionY;
			const float motionZf = (float)motionZ;
			float f = MathHelper::sqrt_float(motionXf * motionXf * 0.2f + motionYf * motionYf + motionZf * motionZf * 0.2f) * 0.2f;
#else
			float f = MathHelper::sqrt_double(motionX * motionX * static_cast<double>(0.2f) + motionY * motionY + motionZ * motionZ * static_cast<double>(0.2f)) * 0.2f;
#endif
			if (f > 1.0f)
			{
				f = 1.0f;
			}
			worldObj->playSoundAtEntity(this, "random.splash", f, 1.0f + rand.nextFloatDifference() * 0.4f);
			float f1 = (float)MathHelper::floor_double(boundingBox->minY);
			for (int_t i = 0; (float)i < 1.0f + width * 20.0f; i++)
			{
				float f2 = (rand.nextFloat() * 2.0f - 1.0f) * width;
				float f4 = (rand.nextFloat() * 2.0f - 1.0f) * width;
				worldObj->spawnParticle("bubble", posX + (double)f2, f1 + 1.0f, posZ + (double)f4, motionX, motionY - (double)(rand.nextFloat() * 0.2f), motionZ);
			}
			for (int_t j = 0; (float)j < 1.0f + width * 20.0f; j++)
			{
				float f3 = (rand.nextFloat() * 2.0f - 1.0f) * width;
				float f5 = (rand.nextFloat() * 2.0f - 1.0f) * width;
				worldObj->spawnParticle("splash", posX + (double)f3, f1 + 1.0f, posZ + (double)f5, motionX, motionY, motionZ);
			}
		}
		fallDistance = 0.0f;
		inWater = true;
		fire = 0;
	}
	else
	{
		inWater = false;
	}
	if (worldObj->multiplayerWorld)
	{
		fire = 0;
	}
	else if (fire > 0)
	{
		if (immuneToFire)
		{
			fire -= 4;
			if (fire < 0)
			{
				fire = 0;
			}
		}
		else
		{
			if (fire % 20 == 0)
			{
				attackEntityFrom(DamageSource::onFire, 1);
			}
			fire--;
		}
	}
	if (handleLavaMovement())
	{
		setOnFireFromLava();
		fallDistance *= 0.5f;
	}
	if (posY < -64.0)
	{
		kill();
	}
	if (!worldObj->multiplayerWorld)
	{
		setEntityFlag(0, fire > 0);
		setEntityFlag(2, ridingEntity != nullptr);
	}
	isFirstUpdate = false;
}

void Entity::setOnFireFromLava()
{
	if (!immuneToFire)
	{
		attackEntityFrom(DamageSource::lava, 4);
		setFire(15);
	}
}

void Entity::kill()
{
	setEntityDead();
}

bool Entity::isOffsetPositionInLiquid(double d, double d1, double d2)
{
	AxisAlignedBB *axisalignedbb = boundingBox->getOffsetBoundingBox(d, d1, d2);
	if (hasWorldCollision(worldObj, this, axisalignedbb))
	{
		return false;
	}
	return !worldObj->getIsAnyLiquid(axisalignedbb);
}

void Entity::moveEntity(double d, double d1, double d2)
{
	if (noClip)
	{
		boundingBox->offset(d, d1, d2);
		posX = (boundingBox->minX + boundingBox->maxX) / 2.0;
		posY = (boundingBox->minY + (double)yOffset) - (double)ySize;
		posZ = (boundingBox->minZ + boundingBox->maxZ) / 2.0;
		if (worldObj != nullptr && worldObj->isLimitedWorld())
		{
			constexpr double BOUNDARY = 127.5;
			if (posX < -BOUNDARY) posX = -BOUNDARY;
			else if (posX > BOUNDARY) posX = BOUNDARY;
			if (posZ < -BOUNDARY) posZ = -BOUNDARY;
			else if (posZ > BOUNDARY) posZ = BOUNDARY;
		}
		return;
	}
	ySize *= 0.4f;
	double d3 = posX;
	double d4 = posZ;
	if (isInWeb)
	{
		isInWeb = false;
		d *= 0.25;
		d1 *= 0.05000000074505806;
		d2 *= 0.25;
		motionX = 0.0;
		motionY = 0.0;
		motionZ = 0.0;
	}
	double d5 = d;
	double d6 = d1;
	double d7 = d2;
	AxisAlignedBB *axisalignedbb = boundingBox->copy();
	bool flag = onGround && isSneaking() && isPlayer();
	if (flag)
	{
		double d8 = 0.05;
		for (; d != 0.0 && !hasWorldCollision(worldObj, this, boundingBox->getOffsetBoundingBox(d, -1.0, 0.0)); d5 = d)
		{
			if (d < d8 && d >= -d8)
			{
				d = 0.0;
				continue;
			}
			if (d > 0.0)
			{
				d -= d8;
			}
			else
			{
				d += d8;
			}
		}
		for (; d2 != 0.0 && !hasWorldCollision(worldObj, this, boundingBox->getOffsetBoundingBox(0.0, -1.0, d2)); d7 = d2)
		{
			if (d2 < d8 && d2 >= -d8)
			{
				d2 = 0.0;
				continue;
			}
			if (d2 > 0.0)
			{
				d2 -= d8;
			}
			else
			{
				d2 += d8;
			}
		}
	}
#if PLATFORM_FLOAT_COLLISION_SWEEP
	// Same statement order as the double path below, including where flag1 is
	// sampled (before the X reset can zero d1); see the sweep helpers above.
	worldObj->collectCollisionSweep(this, boundingBox->addCoord(d, d1, d2), s_collisionSweep);
	SweepLocalBox sweepBox;
	sweepRebase(boundingBox, s_collisionSweep, sweepBox);
	sweepApplyY(s_collisionSweep, boundingBox, sweepBox, d1);
	if (!field_9293_aM && d6 != d1)
	{
		d = d1 = d2 = 0.0;
	}
	bool flag1 = onGround || (d6 != d1 && d6 < 0.0);
	sweepApplyX(s_collisionSweep, boundingBox, sweepBox, d);
	if (!field_9293_aM && d5 != d)
	{
		d = d1 = d2 = 0.0;
	}
	sweepApplyZ(s_collisionSweep, boundingBox, sweepBox, d2);
	if (!field_9293_aM && d7 != d2)
	{
		d = d1 = d2 = 0.0;
	}
#else
	const std::vector<AxisAlignedBB *> &list = worldObj->getCollidingBoundingBoxes(this, boundingBox->addCoord(d, d1, d2));
	for (size_t i = 0; i < list.size(); i++)
	{
		d1 = list[i]->calculateYOffset(boundingBox, d1);
	}
	boundingBox->offset(0.0, d1, 0.0);
	if (!field_9293_aM && d6 != d1)
	{
		d = d1 = d2 = 0.0;
	}
	bool flag1 = onGround || (d6 != d1 && d6 < 0.0);
	for (size_t j = 0; j < list.size(); j++)
	{
		d = list[j]->calculateXOffset(boundingBox, d);
	}
	boundingBox->offset(d, 0.0, 0.0);
	if (!field_9293_aM && d5 != d)
	{
		d = d1 = d2 = 0.0;
	}
	for (size_t k = 0; k < list.size(); k++)
	{
		d2 = list[k]->calculateZOffset(boundingBox, d2);
	}
	boundingBox->offset(0.0, 0.0, d2);
	if (!field_9293_aM && d7 != d2)
	{
		d = d1 = d2 = 0.0;
	}
#endif
	if (stepHeight > 0.0f && flag1 && (flag || ySize < 0.05f) && (d5 != d || d7 != d2))
	{
		double d9 = d;
		double d11 = d1;
		double d13 = d2;
		d = d5;
		d1 = stepHeight;
		d2 = d7;
		AxisAlignedBB *axisalignedbb1 = boundingBox->copy();
		boundingBox->setBB(axisalignedbb);
#if PLATFORM_FLOAT_COLLISION_SWEEP
		worldObj->collectCollisionSweep(this, boundingBox->addCoord(d, d1, d2), s_collisionSweep);
		sweepRebase(boundingBox, s_collisionSweep, sweepBox);
		sweepApplyY(s_collisionSweep, boundingBox, sweepBox, d1);
		if (!field_9293_aM && d6 != d1)
		{
			d = d1 = d2 = 0.0;
		}
		sweepApplyX(s_collisionSweep, boundingBox, sweepBox, d);
		if (!field_9293_aM && d5 != d)
		{
			d = d1 = d2 = 0.0;
		}
		sweepApplyZ(s_collisionSweep, boundingBox, sweepBox, d2);
		if (!field_9293_aM && d7 != d2)
		{
			d = d1 = d2 = 0.0;
		}
		if (!field_9293_aM && d6 != d1)
		{
			d = d1 = d2 = 0.0;
		}
		else
		{
			d1 = -stepHeight;
			sweepApplyY(s_collisionSweep, boundingBox, sweepBox, d1);
		}
#else
		const std::vector<AxisAlignedBB *> &list1 = worldObj->getCollidingBoundingBoxes(this, boundingBox->addCoord(d, d1, d2));
		for (size_t j2 = 0; j2 < list1.size(); j2++)
		{
			d1 = list1[j2]->calculateYOffset(boundingBox, d1);
		}
		boundingBox->offset(0.0, d1, 0.0);
		if (!field_9293_aM && d6 != d1)
		{
			d = d1 = d2 = 0.0;
		}
		for (size_t k2 = 0; k2 < list1.size(); k2++)
		{
			d = list1[k2]->calculateXOffset(boundingBox, d);
		}
		boundingBox->offset(d, 0.0, 0.0);
		if (!field_9293_aM && d5 != d)
		{
			d = d1 = d2 = 0.0;
		}
		for (size_t l2 = 0; l2 < list1.size(); l2++)
		{
			d2 = list1[l2]->calculateZOffset(boundingBox, d2);
		}
		boundingBox->offset(0.0, 0.0, d2);
		if (!field_9293_aM && d7 != d2)
		{
			d = d1 = d2 = 0.0;
		}
		if (!field_9293_aM && d6 != d1)
		{
			d = d1 = d2 = 0.0;
		}
		else
		{
			d1 = -stepHeight;
			for (size_t i3 = 0; i3 < list1.size(); i3++)
			{
				d1 = list1[i3]->calculateYOffset(boundingBox, d1);
			}
			boundingBox->offset(0.0, d1, 0.0);
		}
#endif
		if (d9 * d9 + d13 * d13 >= d * d + d2 * d2)
		{
			d = d9;
			d1 = d11;
			d2 = d13;
			boundingBox->setBB(axisalignedbb1);
		}
		else
		{
			double d14 = boundingBox->minY - (double)JavaArithmetic::doubleToInt(boundingBox->minY);
			if (d14 > 0.0)
			{
				ySize += (float)(d14 + 0.01);
			}
		}
	}
	posX = (boundingBox->minX + boundingBox->maxX) / 2.0;
	posY = (boundingBox->minY + (double)yOffset) - (double)ySize;
	posZ = (boundingBox->minZ + boundingBox->maxZ) / 2.0;
	if (worldObj != nullptr && worldObj->isLimitedWorld())
	{
		constexpr double BOUNDARY = 127.5;
		double clampedX = posX;
		double clampedZ = posZ;
		if (clampedX < -BOUNDARY) { clampedX = -BOUNDARY; motionX = 0.0; isCollidedHorizontally = true; }
		else if (clampedX > BOUNDARY) { clampedX = BOUNDARY; motionX = 0.0; isCollidedHorizontally = true; }
		if (clampedZ < -BOUNDARY) { clampedZ = -BOUNDARY; motionZ = 0.0; isCollidedHorizontally = true; }
		else if (clampedZ > BOUNDARY) { clampedZ = BOUNDARY; motionZ = 0.0; isCollidedHorizontally = true; }
		if (clampedX != posX || clampedZ != posZ)
		{
			boundingBox->offset(clampedX - posX, 0.0, clampedZ - posZ);
			posX = clampedX;
			posZ = clampedZ;
		}
	}
	isCollidedHorizontally = d5 != d || d7 != d2;
	isCollidedVertically = d6 != d1;
	onGround = d6 != d1 && d6 < 0.0;
	isCollided = isCollidedHorizontally || isCollidedVertically;
	updateFallState(d1, onGround);
	if (d5 != d)
	{
		motionX = 0.0;
	}
	if (d6 != d1)
	{
		motionY = 0.0;
	}
	if (d7 != d2)
	{
		motionZ = 0.0;
	}
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
	const float d10 = (float)(posX - d3);
	const float d12 = (float)(posZ - d4);
#else
	double d10 = posX - d3;
	double d12 = posZ - d4;
#endif
	if (canTriggerWalking() && !flag && ridingEntity == nullptr)
	{
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
		distanceWalkedModified += MathHelper::sqrt_float(d10 * d10 + d12 * d12) * 0.6f;
#else
		distanceWalkedModified += (float)((double)MathHelper::sqrt_double(d10 * d10 + d12 * d12) * 0.6);
#endif
		int_t l = MathHelper::floor_double(posX);
		int_t j1 = MathHelper::floor_double(posY - 0.20000000298023224 - (double)yOffset);
		int_t l1 = MathHelper::floor_double(posZ);
		int_t j3 = worldObj->getBlockId(l, j1, l1);
		const int_t belowY = JavaArithmetic::intSub(j1, 1);
		if (j3 == 0 && worldObj->getBlockId(l, belowY, l1) == Block::fence->blockID)
		{
			j3 = worldObj->getBlockId(l, belowY, l1);
		}
		if (distanceWalkedModified > (float)nextStepDistance && j3 > 0)
		{
			nextStepDistance = JavaArithmetic::intAdd(JavaArithmetic::floatToInt(distanceWalkedModified), 1);
			playStepSound(l, j1, l1, j3);
			Block::blocksList[j3]->onEntityWalking(worldObj, l, j1, l1, this);
		}
	}
	int_t i1 = MathHelper::floor_double(boundingBox->minX + 0.001);
	int_t k1 = MathHelper::floor_double(boundingBox->minY + 0.001);
	int_t i2 = MathHelper::floor_double(boundingBox->minZ + 0.001);
	int_t k3 = MathHelper::floor_double(boundingBox->maxX - 0.001);
	int_t l3 = MathHelper::floor_double(boundingBox->maxY - 0.001);
	int_t i4 = MathHelper::floor_double(boundingBox->maxZ - 0.001);
	if (worldObj->checkChunksExist(i1, k1, i2, k3, l3, i4))
	{
		for (int_t j4 = i1; j4 <= k3; j4++)
		{
			for (int_t k4 = k1; k4 <= l3; k4++)
			{
				for (int_t l4 = i2; l4 <= i4; l4++)
				{
					int_t i5 = worldObj->getBlockId(j4, k4, l4);
					if (i5 > 0)
					{
						Block::blocksList[i5]->onEntityCollidedWithBlock(worldObj, j4, k4, l4, this);
					}
				}
			}
		}
	}
	bool flag2 = isWet();
	if (worldObj->isBoundingBoxBurning(boundingBox->contract(0.001, 0.001, 0.001)))
	{
		dealFireDamage(1);
		if (!flag2)
		{
			fire++;
			if (fire == 0)
			{
				setFire(8);
			}
		}
	}
	else if (fire <= 0)
	{
		fire = -fireResistance;
	}
	if (flag2 && fire > 0)
	{
		worldObj->playSoundAtEntity(this, "random.fizz", 0.7f, 1.6f + rand.nextFloatDifference() * 0.4f);
		fire = -fireResistance;
	}
}

void Entity::playStepSound(int_t x, int_t y, int_t z, int_t blockId)
{
	if (worldObj == nullptr || blockId <= 0 || blockId >= Block::BLOCK_REGISTRY_SIZE || Block::blocksList[blockId] == nullptr)
		return;
	StepSound *stepSound = Block::blocksList[blockId]->stepSound;
	if (Block::snow != nullptr && worldObj->getBlockId(x, y + 1, z) == Block::snow->blockID)
		stepSound = Block::snow->stepSound;
	else if (Block::blocksList[blockId]->blockMaterial->getIsLiquid())
		return;
	worldObj->playSoundAtEntity(this, stepSound->getStepSound(), stepSound->getVolume() * 0.15f, stepSound->getPitch());
}

bool Entity::canTriggerWalking()
{
	return true;
}

void Entity::updateFallState(double d, bool flag)
{
	if (flag)
	{
		if (fallDistance > 0.0f)
		{
			if (isLiving())
			{
				int_t x = MathHelper::floor_double(posX);
				int_t y = MathHelper::floor_double(posY - 0.20000000298023224 - (double)yOffset);
				int_t z = MathHelper::floor_double(posZ);
				int_t blockId = worldObj->getBlockId(x, y, z);
				if (blockId == 0 && worldObj->getBlockId(x, y - 1, z) == Block::fence->blockID)
					blockId = worldObj->getBlockId(x, y - 1, z);

				if (blockId > 0)
					Block::blocksList[blockId]->onFallenUpon(worldObj, x, y, z, this, fallDistance);
			}

			fall(fallDistance);
			fallDistance = 0.0f;
		}
	}
	else if (d < 0.0)
	{
		fallDistance -= (float)d;
	}
}

AxisAlignedBB *Entity::getBoundingBox()
{
	return nullptr;
}

void Entity::dealFireDamage(int_t i)
{
	if (!immuneToFire)
	{
		attackEntityFrom(DamageSource::inFire, i);
	}
}

void Entity::fall(float f)
{
	if (riddenByEntity != nullptr)
	{
		riddenByEntity->fall(f);
	}
}

bool Entity::isWet()
{
	return inWater || worldObj->canBlockBeRainedOn(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ));
}

void Entity::setFire(int_t seconds)
{
	const int_t ticks = seconds * 20;
	if (fire < ticks)
		fire = ticks;
}

void Entity::extinguish()
{
	fire = 0;
}

bool Entity::isInWater()
{
	return inWater;
}

bool Entity::handleWaterMovement()
{
	return worldObj->handleMaterialAcceleration(boundingBox->expand(0.0, static_cast<double>(-0.4f), 0.0)->contract(0.001, 0.001, 0.001), Material::water, this);
}

bool Entity::isInsideOfMaterial(Material *material)
{
	double d = posY + (double)getEyeHeight();
	int_t i = MathHelper::floor_double(posX);
	int_t j = MathHelper::floor_float((float)MathHelper::floor_double(d));
	int_t k = MathHelper::floor_double(posZ);
	int_t l = worldObj->getBlockId(i, j, k);
	if (l != 0 && Block::blocksList[l]->blockMaterial == material)
	{
		float f = BlockFluid::getPercentAir(worldObj->getBlockMetadata(i, j, k)) - 1.0f / 9.0f;
		float f1 = static_cast<float>(JavaArithmetic::intAdd(j, 1)) - f;
		return d < (double)f1;
	}
	return false;
}

float Entity::getEyeHeight()
{
	return 0.0f;
}

bool Entity::handleLavaMovement()
{
	return worldObj->isMaterialInBB(boundingBox->expand(-0.10000000149011612, -0.40000000596046448, -0.10000000149011612), Material::lava);
}

void Entity::moveFlying(float f, float f1, float f2)
{
	float f3 = MathHelper::sqrt_float(f * f + f1 * f1);
	if (f3 < 0.01f)
	{
		return;
	}
	if (f3 < 1.0f)
	{
		f3 = 1.0f;
	}
	f3 = f2 / f3;
	f *= f3;
	f1 *= f3;
	float f4 = MathHelper::sin((rotationYaw * 3.1415927f) / 180.0f);
	float f5 = MathHelper::cos((rotationYaw * 3.1415927f) / 180.0f);
	motionX += f * f5 - f1 * f4;
	motionZ += f1 * f5 + f * f4;
}

int_t Entity::getBrightnessForRender(float)
{
	const int_t x = MathHelper::floor_double(posX);
	const int_t z = MathHelper::floor_double(posZ);
	if (!worldObj->blockExists(x, 0, z))
		return 0;

	const double verticalSample = (boundingBox->maxY - boundingBox->minY) * 0.66;
	const int_t y = MathHelper::floor_double(posY - static_cast<double>(yOffset) + verticalSample);
	return worldObj->getLightBrightnessForSkyBlocks(x, y, z, 0);
}

float Entity::getEntityBrightness(float)
{
	const int_t i = MathHelper::floor_double(posX);
	const int_t k = MathHelper::floor_double(posZ);
	if (!worldObj->blockExists(i, 0, k))
		return 0.0f;

	const double verticalSample = (boundingBox->maxY - boundingBox->minY) * 0.66;
	const int_t j = MathHelper::floor_double(posY - static_cast<double>(yOffset) + verticalSample);
	return worldObj->getLightBrightness(i, j, k);
}

int_t Entity::getAir() const
{
	return dataWatcher != nullptr && dataWatcher->hasObject(1)
		? (int_t)dataWatcher->getWatchableObjectShort(1)
		: air;
}

void Entity::setAir(int_t value)
{
	air = value;
	if (dataWatcher != nullptr && dataWatcher->hasObject(1))
		dataWatcher->updateObject(1, (short_t)value);
}

void Entity::setInWeb()
{
	isInWeb = true;
	fallDistance = 0.0f;
}

void Entity::setWorld(World *world)
{
	worldObj = world;
#if PLATFORM_CACHE_ENTITY_CHUNK_EXISTENCE
	invalidateChunkExistenceCache();
#endif
}

void Entity::setPositionAndRotation(double d, double d1, double d2, float f, float f1)
{
	prevPosX = posX = d;
	prevPosY = posY = d1;
	prevPosZ = posZ = d2;
	prevRotationYaw = rotationYaw = f;
	prevRotationPitch = rotationPitch = f1;
	ySize = 0.0f;
	double d3 = (double)prevRotationYaw - (double)f;
	if (d3 < -180.0)
	{
		prevRotationYaw += 360.0f;
	}
	if (d3 >= 180.0)
	{
		prevRotationYaw -= 360.0f;
	}
	setPosition(posX, posY, posZ);
	setRotation(f, f1);
}

void Entity::turnEntity(float yaw, float pitch)
{
	float prevYaw = rotationYaw;
	float prevPitch = rotationPitch;
	rotationYaw   += yaw   * 0.15f;
	rotationPitch -= pitch * 0.15f;
	if (rotationPitch < -90.0f) rotationPitch = -90.0f;
	if (rotationPitch >  90.0f) rotationPitch =  90.0f;
	prevRotationPitch += rotationPitch - prevPitch;
	prevRotationYaw   += rotationYaw   - prevYaw;
}

void Entity::setLocationAndAngles(double d, double d1, double d2, float f, float f1)
{
	lastTickPosX = prevPosX = posX = d;
	lastTickPosY = prevPosY = posY = d1 + (double)yOffset;
	lastTickPosZ = prevPosZ = posZ = d2;
	rotationYaw = f;
	rotationPitch = f1;
	setPosition(posX, posY, posZ);
}

float Entity::getDistanceToEntity(Entity *entity)
{
	float f = (float)(posX - entity->posX);
	float f1 = (float)(posY - entity->posY);
	float f2 = (float)(posZ - entity->posZ);
	return MathHelper::sqrt_float(f * f + f1 * f1 + f2 * f2);
}

double Entity::getDistanceSq(double d, double d1, double d2)
{
#if PLATFORM_FLOAT_ENTITY_DISTANCE
	// Subtract in double so the separation stays exact far from the origin, then
	// square in float -- the same shape getDistanceToEntity above already uses.
	const float d3 = (float)(posX - d);
	const float d4 = (float)(posY - d1);
	const float d5 = (float)(posZ - d2);
	return (double)(d3 * d3 + d4 * d4 + d5 * d5);
#else
	double d3 = posX - d;
	double d4 = posY - d1;
	double d5 = posZ - d2;
	return d3 * d3 + d4 * d4 + d5 * d5;
#endif
}

double Entity::getDistance(double d, double d1, double d2)
{
#if PLATFORM_FLOAT_ENTITY_DISTANCE
	// sqrt_double already truncates its result to float, so narrowing the
	// operands only moves an existing truncation earlier.
	const float d3 = (float)(posX - d);
	const float d4 = (float)(posY - d1);
	const float d5 = (float)(posZ - d2);
	return (double)MathHelper::sqrt_float(d3 * d3 + d4 * d4 + d5 * d5);
#else
	double d3 = posX - d;
	double d4 = posY - d1;
	double d5 = posZ - d2;
	return (double)MathHelper::sqrt_double(d3 * d3 + d4 * d4 + d5 * d5);
#endif
}

double Entity::getDistanceSqToEntity(Entity *entity)
{
#if PLATFORM_FLOAT_ENTITY_DISTANCE
	const float d = (float)(posX - entity->posX);
	const float d1 = (float)(posY - entity->posY);
	const float d2 = (float)(posZ - entity->posZ);
	return (double)(d * d + d1 * d1 + d2 * d2);
#else
	double d = posX - entity->posX;
	double d1 = posY - entity->posY;
	double d2 = posZ - entity->posZ;
	return d * d + d1 * d1 + d2 * d2;
#endif
}

void Entity::onCollideWithPlayer(EntityPlayer *entityplayer)
{
}

void Entity::applyEntityCollision(Entity *entity)
{
	if (entity->riddenByEntity == this || entity->ridingEntity == this)
	{
		return;
	}
	double d = entity->posX - posX;
	double d1 = entity->posZ - posZ;
	double d2 = MathHelper::abs_max(d, d1);
	if (d2 >= 0.0099999997764825821)
	{
		d2 = MathHelper::sqrt_double(d2);
		d /= d2;
		d1 /= d2;
		double d3 = 1.0 / d2;
		if (d3 > 1.0)
		{
			d3 = 1.0;
		}
		d *= d3;
		d1 *= d3;
		d *= 0.05000000074505806;
		d1 *= 0.05000000074505806;
		d *= 1.0f - entityCollisionReduction;
		d1 *= 1.0f - entityCollisionReduction;
		addVelocity(-d, 0.0, -d1);
		entity->addVelocity(d, 0.0, d1);
	}
}

void Entity::addVelocity(double d, double d1, double d2)
{
	motionX += d;
	motionY += d1;
	motionZ += d2;
	isAirBorne = true;
}

void Entity::setBeenAttacked()
{
	velocityChanged = true;
}

bool Entity::attackEntityFrom(Entity *entity, int_t i)
{
	setBeenAttacked();
	return false;
}

bool Entity::attackEntityFrom(const DamageSource &source, int_t damage)
{
	return attackEntityFrom(source.getEntity(), damage);
}

bool Entity::canBeCollidedWith()
{
	return false;
}

bool Entity::canAttackWithItem()
{
	return true;
}

bool Entity::canBePushed()
{
	return false;
}

void Entity::addToPlayerScore(Entity *entity, int_t i)
{
}

bool Entity::isInRangeToRenderVec3D(Vec3D *vec3d)
{
	double d = posX - vec3d->xCoord;
	double d1 = posY - vec3d->yCoord;
	double d2 = posZ - vec3d->zCoord;
	double d3 = d * d + d1 * d1 + d2 * d2;
	return isInRangeToRenderDist(d3);
}

bool Entity::isInRangeToRenderDist(double d)
{
	double d1 = boundingBox->getAverageEdgeLength();
	d1 *= 64.0 * renderDistanceWeight;
	return d < d1 * d1;
}

const char *Entity::getEntityTexture()
{
	return "";
}
const char *Entity::getTexture()
{
	return getEntityTexture();
}

std::vector<Entity *> Entity::getParts() const
{
    return {};
}

bool Entity::isImmuneToFire() const
{
	return immuneToFire;
}


bool Entity::addEntityID(NBTTagCompound *nbttagcompound)
{
	jstring s = getEntityString();
	if (isDead || s.empty())
	{
		return false;
	}
	nbttagcompound->setString("id", s);
	writeToNBT(nbttagcompound);
	return true;
}

void Entity::writeToNBT(NBTTagCompound *nbttagcompound)
{
	double pos[3] = { posX, posY + (double)ySize, posZ };
	double mot[3] = { motionX, motionY, motionZ };
	float rot[2] = { rotationYaw, rotationPitch };
	nbttagcompound->setTag("Pos", newDoubleNBTList(pos, 3));
	nbttagcompound->setTag("Motion", newDoubleNBTList(mot, 3));
	nbttagcompound->setTag("Rotation", newFloatNBTList(rot, 2));
	nbttagcompound->setFloat("FallDistance", fallDistance);
	nbttagcompound->setShort("Fire", JavaArithmetic::shortFromBits(static_cast<ushort_t>(fire)));
	nbttagcompound->setShort("Air", JavaArithmetic::shortFromBits(static_cast<ushort_t>(getAir())));
	nbttagcompound->setBoolean("OnGround", onGround);
	writeEntityToNBT(nbttagcompound);
}

void Entity::readFromNBT(NBTTagCompound *nbttagcompound)
{
	NBTTagList *nbttaglist  = nbttagcompound->getTagList("Pos");
	NBTTagList *nbttaglist1 = nbttagcompound->getTagList("Motion");
	NBTTagList *nbttaglist2 = nbttagcompound->getTagList("Rotation");
	motionX = ((NBTTagDouble *)nbttaglist1->tagAt(0))->doubleValue;
	motionY = ((NBTTagDouble *)nbttaglist1->tagAt(1))->doubleValue;
	motionZ = ((NBTTagDouble *)nbttaglist1->tagAt(2))->doubleValue;
	if (std::abs(motionX) > 10.0) { motionX = 0.0; }
	if (std::abs(motionY) > 10.0) { motionY = 0.0; }
	if (std::abs(motionZ) > 10.0) { motionZ = 0.0; }
	prevPosX = lastTickPosX = posX = ((NBTTagDouble *)nbttaglist->tagAt(0))->doubleValue;
	prevPosY = lastTickPosY = posY = ((NBTTagDouble *)nbttaglist->tagAt(1))->doubleValue;
	prevPosZ = lastTickPosZ = posZ = ((NBTTagDouble *)nbttaglist->tagAt(2))->doubleValue;
	prevRotationYaw   = rotationYaw   = ((NBTTagFloat *)nbttaglist2->tagAt(0))->floatValue;
	prevRotationPitch = rotationPitch = ((NBTTagFloat *)nbttaglist2->tagAt(1))->floatValue;
	fallDistance = nbttagcompound->getFloat("FallDistance");
	fire = nbttagcompound->getShort("Fire");
	setAir(nbttagcompound->getShort("Air"));
	onGround = nbttagcompound->getBoolean("OnGround");
	setPosition(posX, posY, posZ);
	setRotation(rotationYaw, rotationPitch);
	readEntityFromNBT(nbttagcompound);
}

jstring Entity::getEntityString()
{
	return EntityList::getEntityString(this);
}

NBTTagList *Entity::newDoubleNBTList(const double *ad, int_t count)
{
	NBTTagList *nbttaglist = new NBTTagList();
	for (int_t j = 0; j < count; j++)
	{
		nbttaglist->setTag(new NBTTagDouble(ad[j]));
	}
	return nbttaglist;
}

NBTTagList *Entity::newFloatNBTList(const float *af, int_t count)
{
	NBTTagList *nbttaglist = new NBTTagList();
	for (int_t j = 0; j < count; j++)
	{
		nbttaglist->setTag(new NBTTagFloat(af[j]));
	}
	return nbttaglist;
}

float Entity::getShadowSize()
{
	return height / 2.0f;
}

EntityItem *Entity::dropItem(int_t i, int_t j)
{
	return dropItemWithOffset(i, j, 0.0f);
}

EntityItem *Entity::dropItemWithOffset(int_t i, int_t j, float f)
{
	return entityDropItem(new ItemStack(i, j, 0), f);
}

EntityItem *Entity::entityDropItem(ItemStack *itemstack, float f)
{
	EntityItem *entityitem = new EntityItem(worldObj, posX, posY + (double)f, posZ, itemstack);
	entityitem->delayBeforeCanPickup = 10;
	if (!worldObj->entityJoinedWorld(entityitem))
	{
		delete entityitem;
		return nullptr;
	}
	return entityitem;
}

bool Entity::isEntityAlive()
{
	return !isDead;
}

bool Entity::isEntityInsideOpaqueBlock()
{
	for (int_t i = 0; i < 8; i++)
	{
		float f  = ((float)((i >> 0) % 2) - 0.5f) * width * 0.8f;
		float f1 = ((float)((i >> 1) % 2) - 0.5f) * 0.1f;
		float f2 = ((float)((i >> 2) % 2) - 0.5f) * width * 0.8f;
		int_t j = MathHelper::floor_double(posX + (double)f);
		int_t k = MathHelper::floor_double(posY + (double)getEyeHeight() + (double)f1);
		int_t l = MathHelper::floor_double(posZ + (double)f2);
		if (worldObj->isBlockNormalCube(j, k, l))
		{
			return true;
		}
	}
	return false;
}

bool Entity::interact(EntityPlayer *entityplayer)
{
	return false;
}

AxisAlignedBB *Entity::getCollisionBox(Entity *entity)
{
	return nullptr;
}

void Entity::updateRidden()
{
	if (ridingEntity->isDead)
	{
		ridingEntity = nullptr;
		return;
	}
	motionX = 0.0;
	motionY = 0.0;
	motionZ = 0.0;
	onUpdate();
	if (ridingEntity == nullptr)
	{
		return;
	}
	ridingEntity->updateRiderPosition();
	entityRiderYawDelta   += (double)(ridingEntity->rotationYaw - ridingEntity->prevRotationYaw);
	entityRiderPitchDelta += (double)(ridingEntity->rotationPitch - ridingEntity->prevRotationPitch);
	for (; entityRiderYawDelta   >= 180.0; entityRiderYawDelta   -= 360.0) {}
	for (; entityRiderYawDelta   < -180.0; entityRiderYawDelta   += 360.0) {}
	for (; entityRiderPitchDelta >= 180.0; entityRiderPitchDelta -= 360.0) {}
	for (; entityRiderPitchDelta < -180.0; entityRiderPitchDelta += 360.0) {}
	double d  = entityRiderYawDelta * 0.5;
	double d1 = entityRiderPitchDelta * 0.5;
	float f = 10.0f;
	if (d  >  (double)f) { d  =  f; }
	if (d  < -(double)f) { d  = -f; }
	if (d1 >  (double)f) { d1 =  f; }
	if (d1 < -(double)f) { d1 = -f; }
	entityRiderYawDelta   -= d;
	entityRiderPitchDelta -= d1;
	rotationYaw   += (float)d;
	rotationPitch += (float)d1;
}

void Entity::updateRiderPosition()
{
	riddenByEntity->setPosition(posX, posY + getMountedYOffset() + riddenByEntity->getYOffset(), posZ);
}

double Entity::getYOffset()
{
	return (double)yOffset;
}

double Entity::getMountedYOffset()
{
	return (double)height * 0.75;
}

void Entity::mountEntity(Entity *entity)
{
	entityRiderPitchDelta = 0.0;
	entityRiderYawDelta   = 0.0;
	if (entity == nullptr)
	{
		if (ridingEntity != nullptr)
		{
			setLocationAndAngles(ridingEntity->posX, ridingEntity->boundingBox->minY + (double)ridingEntity->height, ridingEntity->posZ, rotationYaw, rotationPitch);
			ridingEntity->riddenByEntity = nullptr;
		}
		ridingEntity = nullptr;
		return;
	}
	if (ridingEntity == entity)
	{
		ridingEntity->riddenByEntity = nullptr;
		ridingEntity = nullptr;
		setLocationAndAngles(entity->posX, entity->boundingBox->minY + (double)entity->height, entity->posZ, rotationYaw, rotationPitch);
		return;
	}
	if (ridingEntity != nullptr)
	{
		ridingEntity->riddenByEntity = nullptr;
	}
	if (entity->riddenByEntity != nullptr)
	{
		entity->riddenByEntity->ridingEntity = nullptr;
	}
	ridingEntity = entity;
	entity->riddenByEntity = this;
}

void Entity::setPositionAndRotation2(double d, double d1, double d2, float f, float f1, int_t i)
{
	setPosition(d, d1, d2);
	setRotation(f, f1);
	const std::vector<AxisAlignedBB *> &list = worldObj->getCollidingBoundingBoxes(this, boundingBox->contract(0.03125, 0.0, 0.03125));
	if (list.size() > 0)
	{
		double d3 = 0.0;
		for (size_t j = 0; j < list.size(); j++)
		{
			AxisAlignedBB *axisalignedbb = list[j];
			if (axisalignedbb->maxY > d3)
			{
				d3 = axisalignedbb->maxY;
			}
		}
		d1 += d3 - boundingBox->minY;
		setPosition(d, d1, d2);
	}
}

float Entity::getCollisionBorderSize()
{
	return 0.1f;
}

Vec3D *Entity::getLookVec()
{
	return nullptr;
}

void Entity::setInPortal()
{
}

void Entity::setVelocity(double d, double d1, double d2)
{
	motionX = d;
	motionY = d1;
	motionZ = d2;
}

void Entity::handleHealthUpdate(byte_t byte0)
{
}

void Entity::performHurtAnimation()
{
}

void Entity::updateCloak()
{
}

void Entity::outfitWithItem(int_t i, int_t j, int_t k)
{
}

bool Entity::isBurning()
{
	return fire > 0 || getEntityFlag(0);
}

bool Entity::isRiding()
{
	return ridingEntity != nullptr || getEntityFlag(2);
}

bool Entity::isSneaking()
{
	return getEntityFlag(1);
}

void Entity::setSneaking(bool value)
{
	setEntityFlag(1, value);
}

bool Entity::isSprinting()
{
	return getEntityFlag(3);
}

void Entity::setSprinting(bool value)
{
	setEntityFlag(3, value);
}

bool Entity::isEating()
{
	return getEntityFlag(4);
}

void Entity::setEating(bool value)
{
	setEntityFlag(4, value);
}

bool Entity::getEntityFlag(int_t i)
{
	return (dataWatcher->getWatchableObjectByte(0) & JavaArithmetic::intShl(1, i)) != 0;
}

void Entity::setEntityFlag(int_t i, bool flag)
{
	byte_t byte0 = dataWatcher->getWatchableObjectByte(0);
	if (flag)
	{
		dataWatcher->updateObject(0, (byte_t)(byte0 | JavaArithmetic::intShl(1, i)));
	}
	else
	{
		dataWatcher->updateObject(0, (byte_t)(byte0 & ~JavaArithmetic::intShl(1, i)));
	}
}

void Entity::onStruckByLightning(EntityLightningBolt *entitylightningbolt)
{
	dealFireDamage(5);
	fire++;
	if (fire == 0)
	{
		setFire(8);
	}
}

void Entity::onKillEntity(EntityLiving *entityliving)
{
}

bool Entity::pushOutOfBlocks(double d, double d1, double d2)
{
	int_t i = MathHelper::floor_double(d);
	int_t j = MathHelper::floor_double(d1);
	int_t k = MathHelper::floor_double(d2);
	double d3 = d - (double)i;
	double d4 = d1 - (double)j;
	double d5 = d2 - (double)k;
	if (worldObj->isBlockNormalCube(i, j, k))
	{
		bool flag  = !worldObj->isBlockNormalCube(i - 1, j, k);
		bool flag1 = !worldObj->isBlockNormalCube(i + 1, j, k);
		bool flag2 = !worldObj->isBlockNormalCube(i, j - 1, k);
		bool flag3 = !worldObj->isBlockNormalCube(i, j + 1, k);
		bool flag4 = !worldObj->isBlockNormalCube(i, j, k - 1);
		bool flag5 = !worldObj->isBlockNormalCube(i, j, k + 1);
		byte_t byte0 = -1;
		double d6 = 9999.0;
		if (flag  && d3       < d6) { d6 = d3;       byte0 = 0; }
		if (flag1 && 1.0 - d3 < d6) { d6 = 1.0 - d3; byte0 = 1; }
		if (flag2 && d4       < d6) { d6 = d4;       byte0 = 2; }
		if (flag3 && 1.0 - d4 < d6) { d6 = 1.0 - d4; byte0 = 3; }
		if (flag4 && d5       < d6) { d6 = d5;       byte0 = 4; }
		if (flag5 && 1.0 - d5 < d6) {                byte0 = 5; }
		float f = rand.nextFloat() * 0.2f + 0.1f;
		if (byte0 == 0) { motionX = -f; }
		if (byte0 == 1) { motionX =  f; }
		if (byte0 == 2) { motionY = -f; }
		if (byte0 == 3) { motionY =  f; }
		if (byte0 == 4) { motionZ = -f; }
		if (byte0 == 5) { motionZ =  f; }
	}
	return false;
}
