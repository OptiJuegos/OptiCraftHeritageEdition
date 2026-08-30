#include "EntityCreature.h"

#include "java/Arithmetic.h"
#include "java/Math.h"

#include <cmath>

#include "AxisAlignedBB.h"
#include "Entity.h"
#include "EntityPlayer.h"
#include "MathHelper.h"
#include "PathEntity.h"
#include "platform/PlatformTuning.h"
#include "Vec3D.h"
#include "World.h"



EntityCreature::EntityCreature(World *world)
	: EntityLiving(world)
{
	pathToEntity = nullptr;
	playerToAttack = nullptr;
	hasAttacked = false;
	fleeingTick = 0;
}

EntityCreature::~EntityCreature() = default;

bool EntityCreature::isMovementCeased()
{
	return false;
}

void EntityCreature::updatePlayerActionState()
{
#if PLATFORM_THROTTLE_ENTITY_AI
	const float playerDistanceSq = getCachedNearestPlayerDistanceSq();
	const float farRadius = PLATFORM_ENTITY_AI_FAR_RADIUS_BLOCKS;
	const float nearRadius = PLATFORM_ENTITY_AI_NEAR_RADIUS_BLOCKS;
	int_t aiTickDivisor = 1;
	if (getCachedNearestPlayer() != nullptr)
	{
		if (playerDistanceSq > farRadius * farRadius)
			aiTickDivisor = PLATFORM_ENTITY_AI_FAR_TICK_DIVISOR;
		else if (playerDistanceSq > nearRadius * nearRadius)
			aiTickDivisor = PLATFORM_ENTITY_AI_MID_TICK_DIVISOR;
	}
	if (aiTickDivisor > 1 && ((JavaArithmetic::intAdd(ticksExisted, entityId)) & (aiTickDivisor - 1)) != 0)
		return;
#endif

	if (fleeingTick > 0)
		--fleeingTick;

	hasAttacked = isMovementCeased();
	const float searchRange = 16.0f;
	if (playerToAttack == nullptr)
	{
		playerToAttack = findPlayerToAttack();
		if (playerToAttack != nullptr)
		{
			pathToEntity.reset(worldObj->getPathEntityToEntity(
				this, playerToAttack, searchRange, true, false, false, true));
		}
	}
	else if (!playerToAttack->isEntityAlive())
	{
		playerToAttack = nullptr;
	}
	else
	{
		const float distance = playerToAttack->getDistanceToEntity(this);
		if (canEntityBeSeen(playerToAttack))
			attackEntity(playerToAttack, distance);
		else
			attackBlockedEntity(playerToAttack, distance);
	}

	if (hasAttacked || playerToAttack == nullptr || (pathToEntity != nullptr && rand.nextInt(20) != 0))
	{
		if (!hasAttacked &&
			((pathToEntity == nullptr && rand.nextInt(180) == 0) || rand.nextInt(120) == 0 || fleeingTick > 0) &&
			entityAge < 100)
		{
			updateWanderPath();
		}
	}
	else
	{
		pathToEntity.reset(worldObj->getPathEntityToEntity(
			this, playerToAttack, searchRange, true, false, false, true));
	}

	const int_t currentY = MathHelper::floor_double(boundingBox->minY + 0.5);
	const bool inWater = isInWater();
	const bool inLava = handleLavaMovement();
	rotationPitch = 0.0f;

	if (pathToEntity != nullptr && rand.nextInt(100) != 0)
	{
		double pathX;
		double pathY;
		double pathZ;
		bool hasPathPoint = pathToEntity->getCurrentNodeCoordinates(this, pathX, pathY, pathZ);
#if PLATFORM_FLOAT_ENTITY_AI_MATH
		const float pathReach = width * 2.0f;
		while (hasPathPoint)
		{
			const float dx = (float)(pathX - posX);
			const float dz = (float)(pathZ - posZ);
			if (dx * dx + dz * dz >= pathReach * pathReach)
				break;
#else
		const double pathReach = (double)width * 2.0;
		while (hasPathPoint)
		{
			const double dx = posX - pathX;
			const double dz = posZ - pathZ;
			if (dx * dx + dz * dz >= pathReach * pathReach)
				break;
#endif
			pathToEntity->incrementPathIndex();
			if (pathToEntity->isFinished())
			{
				hasPathPoint = false;
				pathToEntity.reset();
			}
			else
			{
				hasPathPoint = pathToEntity->getCurrentNodeCoordinates(this, pathX, pathY, pathZ);
			}
		}

		isJumping = false;
		if (hasPathPoint)
		{
#if PLATFORM_FLOAT_ENTITY_AI_MATH
			const float dx = (float)(pathX - posX);
			const float dz = (float)(pathZ - posZ);
			const float dy = (float)(pathY - (double)currentY);
			float targetYaw = (std::atan2(dz, dx) * 180.0f) / 3.14159274f - 90.0f;
#else
			const double dx = pathX - posX;
			const double dz = pathZ - posZ;
			const double dy = pathY - (double)currentY;
			float targetYaw = (float)((JavaMath::atan2(dz, dx) * 180.0) / 3.1415927410125732) - 90.0f;
#endif
			float yawDelta = targetYaw - rotationYaw;
			moveForward = moveSpeed;
			while (yawDelta < -180.0f) yawDelta += 360.0f;
			while (yawDelta >= 180.0f) yawDelta -= 360.0f;
			if (yawDelta > 30.0f) yawDelta = 30.0f;
			if (yawDelta < -30.0f) yawDelta = -30.0f;
			rotationYaw += yawDelta;

			if (hasAttacked && playerToAttack != nullptr)
			{
#if PLATFORM_FLOAT_ENTITY_AI_MATH
				const float targetDx = (float)(playerToAttack->posX - posX);
				const float targetDz = (float)(playerToAttack->posZ - posZ);
				const float previousYaw = rotationYaw;
				rotationYaw = (std::atan2(targetDz, targetDx) * 180.0f) / 3.14159274f - 90.0f;
#else
				const double targetDx = playerToAttack->posX - posX;
				const double targetDz = playerToAttack->posZ - posZ;
				const float previousYaw = rotationYaw;
				rotationYaw = (float)((JavaMath::atan2(targetDz, targetDx) * 180.0) / 3.1415927410125732) - 90.0f;
#endif
				const float strafeAngle = ((previousYaw - rotationYaw + 90.0f) * 3.1415927f) / 180.0f;
				moveStrafing = -MathHelper::sin(strafeAngle) * moveForward;
				moveForward = MathHelper::cos(strafeAngle) * moveForward;
			}

			if (dy > 0.0)
				isJumping = true;
		}

		if (playerToAttack != nullptr)
			faceEntity(playerToAttack, 30.0f, 30.0f);
		if (isCollidedHorizontally && !hasPath())
			isJumping = true;
		if (rand.nextFloat() < 0.8f && (inWater || inLava))
			isJumping = true;
	}
	else
	{
		EntityLiving::updatePlayerActionState();
		pathToEntity.reset();
	}
}

void EntityCreature::updateWanderPath()
{
	bool flag = false;
	int_t i = -1;
	int_t j = -1;
	int_t k = -1;
	float f = -99999.0f;
	for (int_t l = 0; l < 10; l++)
	{
		int_t i1 = MathHelper::floor_double((posX + (double)rand.nextInt(13)) - 6.0);
		int_t j1 = MathHelper::floor_double((posY + (double)rand.nextInt(7))  - 3.0);
		int_t k1 = MathHelper::floor_double((posZ + (double)rand.nextInt(13)) - 6.0);
		float f1 = getBlockPathWeight(i1, j1, k1);
		if (f1 > f)
		{
			f = f1;
			i = i1;
			j = j1;
			k = k1;
			flag = true;
		}
	}
	if (flag)
	{
		pathToEntity.reset(worldObj->getEntityPathToXYZ(this, i, j, k, 10.0f, true, false, false, true));
	}
}

void EntityCreature::attackEntity(Entity *entity, float f)
{
}

void EntityCreature::attackBlockedEntity(Entity *entity, float f)
{
}

float EntityCreature::getBlockPathWeight(int_t i, int_t j, int_t k)
{
	return 0.0f;
}

Entity *EntityCreature::findPlayerToAttack()
{
	return nullptr;
}

float EntityCreature::getSpeedModifier()
{
	if (isAIEnabled())
		return 1.0f;

	float speed = EntityLiving::getSpeedModifier();
	if (fleeingTick > 0)
		speed *= 2.0f;
	return speed;
}

bool EntityCreature::getCanSpawnHere()
{
	int_t i = MathHelper::floor_double(posX);
	int_t j = MathHelper::floor_double(boundingBox->minY);
	int_t k = MathHelper::floor_double(posZ);
	return EntityLiving::getCanSpawnHere() && getBlockPathWeight(i, j, k) >= 0.0f;
}

bool EntityCreature::hasPath()
{
	return pathToEntity != nullptr;
}

void EntityCreature::setPathToEntity(PathEntity *pathentity)
{
	pathToEntity.reset(pathentity);
}

Entity *EntityCreature::getTarget()
{
	return playerToAttack;
}

void EntityCreature::setTarget(Entity *entity)
{
	playerToAttack = entity;
}
