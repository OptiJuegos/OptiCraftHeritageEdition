#include "EntityAIFollowOwner.h"

#include "EntityLiving.h"
#include "EntityLookHelper.h"
#include "EntityTameable.h"
#include "MathHelper.h"
#include "PathNavigate.h"
#include "World.h"

EntityAIFollowOwner::EntityAIFollowOwner(EntityTameable *entity, float moveSpeed, float minDist, float maxDist)
	: pet(entity), world(entity != nullptr ? entity->worldObj : nullptr), speed(moveSpeed),
	  navigator(entity != nullptr ? entity->getNavigator() : nullptr), ownerEntityId(-1), pathRecalcTimer(0),
	  minDistance(minDist), maxDistance(maxDist), oldAvoidsWater(false)
{
	setMutexBits(3);
}

EntityLiving *EntityAIFollowOwner::resolveOwner() const
{
	if (world == nullptr || ownerEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityLiving *>(world->getEntityByID(ownerEntityId));
}

bool EntityAIFollowOwner::shouldExecute()
{
	ownerEntityId = -1;
	if (pet == nullptr)
		return false;
	EntityLiving *owner = pet->getOwner();
	if (owner == nullptr || pet->isSitting() || pet->getDistanceSqToEntity(owner) < (double)(minDistance * minDistance))
		return false;
	ownerEntityId = owner->entityId;
	return true;
}

bool EntityAIFollowOwner::continueExecuting()
{
	EntityLiving *owner = resolveOwner();
	return pet != nullptr && owner != nullptr && owner->isEntityAlive() && navigator != nullptr && !navigator->noPath() &&
		pet->getDistanceSqToEntity(owner) > (double)(maxDistance * maxDistance) && !pet->isSitting();
}

void EntityAIFollowOwner::startExecuting()
{
	pathRecalcTimer = 0;
	if (navigator != nullptr)
	{
		oldAvoidsWater = navigator->getAvoidsWater();
		navigator->setAvoidsWater(false);
	}
}

void EntityAIFollowOwner::resetTask()
{
	ownerEntityId = -1;
	if (navigator != nullptr)
	{
		navigator->clearPathEntity();
		navigator->setAvoidsWater(oldAvoidsWater);
	}
}

void EntityAIFollowOwner::updateTask()
{
	EntityLiving *owner = resolveOwner();
	if (pet == nullptr || owner == nullptr)
		return;
	if (pet->getLookHelper() != nullptr)
		pet->getLookHelper()->setLookPositionWithEntity(owner, 10.0f, (float)pet->getVerticalFaceSpeed());
	if (pet->isSitting() || navigator == nullptr)
		return;
	if (--pathRecalcTimer > 0)
		return;
	pathRecalcTimer = 10;
	if (navigator->tryMoveToEntityLiving(owner, speed) || pet->getDistanceSqToEntity(owner) < 144.0)
		return;

	int_t baseX = MathHelper::floor_double(owner->posX) - 2;
	int_t baseZ = MathHelper::floor_double(owner->posZ) - 2;
	int_t baseY = MathHelper::floor_double(owner->boundingBox->minY);
	for (int_t x = 0; x <= 4; ++x)
	{
		for (int_t z = 0; z <= 4; ++z)
		{
			if ((x >= 1 && z >= 1 && x <= 3 && z <= 3) || world == nullptr)
				continue;
			if (world->isBlockNormalCube(baseX + x, baseY - 1, baseZ + z) &&
				!world->isBlockNormalCube(baseX + x, baseY, baseZ + z) &&
				!world->isBlockNormalCube(baseX + x, baseY + 1, baseZ + z))
			{
				pet->setLocationAndAngles((double)((float)(baseX + x) + 0.5f), (double)baseY,
					(double)((float)(baseZ + z) + 0.5f), pet->rotationYaw, pet->rotationPitch);
				navigator->clearPathEntity();
				return;
			}
		}
	}
}
