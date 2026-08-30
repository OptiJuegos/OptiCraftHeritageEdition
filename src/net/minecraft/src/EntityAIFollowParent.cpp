#include "EntityAIFollowParent.h"

#include <limits>

#include "AxisAlignedBB.h"
#include "Entity.h"
#include "EntityAnimal.h"
#include "PathNavigate.h"
#include "World.h"

EntityAIFollowParent::EntityAIFollowParent(EntityAnimal *child, float speed)
	: childAnimal(child), parentEntityId(-1), moveSpeed(speed), delayCounter(0)
{
}

EntityAnimal *EntityAIFollowParent::resolveParent() const
{
	if (childAnimal == nullptr || childAnimal->worldObj == nullptr || parentEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityAnimal *>(childAnimal->worldObj->getEntityByID(parentEntityId));
}

bool EntityAIFollowParent::shouldExecute()
{
	parentEntityId = -1;
	if (childAnimal == nullptr || childAnimal->worldObj == nullptr || childAnimal->getGrowingAge() >= 0)
		return false;

	const std::vector<Entity *> &entities = childAnimal->worldObj->getEntitiesWithinAABB(typeid(*childAnimal),
		childAnimal->boundingBox->expand(8.0, 4.0, 8.0));
	EntityAnimal *closest = nullptr;
	double closestDistance = std::numeric_limits<double>::max();
	for (Entity *entity : entities)
	{
		EntityAnimal *candidate = dynamic_cast<EntityAnimal *>(entity);
		if (candidate == nullptr || candidate == childAnimal || candidate->getGrowingAge() < 0)
			continue;
		double distance = childAnimal->getDistanceSqToEntity(candidate);
		if (distance <= closestDistance)
		{
			closestDistance = distance;
			closest = candidate;
		}
	}

	if (closest == nullptr || closestDistance < 9.0)
		return false;
	parentEntityId = closest->entityId;
	return true;
}

bool EntityAIFollowParent::continueExecuting()
{
	EntityAnimal *parent = resolveParent();
	if (childAnimal == nullptr || parent == nullptr || !parent->isEntityAlive())
		return false;
	double distance = childAnimal->getDistanceSqToEntity(parent);
	return distance >= 9.0 && distance <= 256.0;
}

void EntityAIFollowParent::startExecuting()
{
	delayCounter = 0;
}

void EntityAIFollowParent::resetTask()
{
	parentEntityId = -1;
}

void EntityAIFollowParent::updateTask()
{
	EntityAnimal *parent = resolveParent();
	if (childAnimal == nullptr || parent == nullptr || childAnimal->getNavigator() == nullptr)
		return;
	if (--delayCounter <= 0)
	{
		delayCounter = 10;
		childAnimal->getNavigator()->tryMoveToEntityLiving(parent, moveSpeed);
	}
}
