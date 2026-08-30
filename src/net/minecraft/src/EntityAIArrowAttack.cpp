#include "EntityAIArrowAttack.h"

#include <algorithm>

#include "EntityArrow.h"
#include "EntityLiving.h"
#include "EntityLookHelper.h"
#include "EntitySenses.h"
#include "EntitySnowball.h"
#include "MathHelper.h"
#include "PathNavigate.h"
#include "World.h"

EntityAIArrowAttack::EntityAIArrowAttack(EntityLiving *host, float speed, int_t attackId, int_t maxAttackTime)
	: worldObj(host != nullptr ? host->worldObj : nullptr), entityHost(host), targetEntityId(-1), rangedAttackTime(0),
	  moveSpeed(speed), seeTime(0), rangedAttackId(attackId), maxRangedAttackTime(maxAttackTime)
{
	setMutexBits(3);
}

EntityLiving *EntityAIArrowAttack::resolveTarget() const
{
	if (worldObj == nullptr || targetEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityLiving *>(worldObj->getEntityByID(targetEntityId));
}

bool EntityAIArrowAttack::shouldExecute()
{
	if (entityHost == nullptr)
		return false;
	EntityLiving *target = entityHost->getAttackTarget();
	if (target == nullptr)
		return false;
	targetEntityId = target->entityId;
	return true;
}

bool EntityAIArrowAttack::continueExecuting()
{
	return shouldExecute() ||
		(entityHost != nullptr && entityHost->getNavigator() != nullptr && !entityHost->getNavigator()->noPath());
}

void EntityAIArrowAttack::resetTask()
{
	targetEntityId = -1;
}

void EntityAIArrowAttack::updateTask()
{
	EntityLiving *target = resolveTarget();
	if (entityHost == nullptr || target == nullptr || entityHost->getNavigator() == nullptr)
		return;
	double distanceSq = entityHost->getDistanceSq(target->posX, target->boundingBox->minY, target->posZ);
	EntitySenses *senses = entityHost->getEntitySenses();
	bool canSee = senses != nullptr && senses->canSee(target);
	if (canSee)
		++seeTime;
	else
		seeTime = 0;
	if (distanceSq <= 100.0 && seeTime >= 20)
		entityHost->getNavigator()->clearPathEntity();
	else
		entityHost->getNavigator()->tryMoveToEntityLiving(target, moveSpeed);
	if (entityHost->getLookHelper() != nullptr)
		entityHost->getLookHelper()->setLookPositionWithEntity(target, 30.0f, 30.0f);
	rangedAttackTime = std::max(rangedAttackTime - 1, 0);
	if (rangedAttackTime <= 0 && distanceSq <= 100.0 && canSee)
	{
		doRangedAttack(target);
		rangedAttackTime = maxRangedAttackTime;
	}
}

void EntityAIArrowAttack::doRangedAttack(EntityLiving *target)
{
	if (worldObj == nullptr || entityHost == nullptr || target == nullptr)
		return;
	if (rangedAttackId == 1)
	{
		EntityArrow *arrow = new EntityArrow(worldObj, entityHost, target, 1.6f, 12.0f);
		worldObj->playSoundAtEntity(entityHost, "random.bow", 1.0f, 1.0f / (entityHost->getRNG().nextFloat() * 0.4f + 0.8f));
		if (!worldObj->entityJoinedWorld(arrow))
			delete arrow;
	}
	else if (rangedAttackId == 2)
	{
		EntitySnowball *snowball = new EntitySnowball(worldObj, entityHost);
		double dx = target->posX - entityHost->posX;
		double dy = target->posY + (double)target->getEyeHeight() - static_cast<double>(1.1f) - snowball->posY;
		double dz = target->posZ - entityHost->posZ;
		float arc = MathHelper::sqrt_double(dx * dx + dz * dz) * 0.2f;
		snowball->setSnowballHeading(dx, dy + (double)arc, dz, 1.6f, 12.0f);
		worldObj->playSoundAtEntity(entityHost, "random.bow", 1.0f, 1.0f / (entityHost->getRNG().nextFloat() * 0.4f + 0.8f));
		if (!worldObj->entityJoinedWorld(snowball))
			delete snowball;
	}
}
