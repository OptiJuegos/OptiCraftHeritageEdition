#include "EntityAIMate.h"

#include "AxisAlignedBB.h"
#include "Entity.h"
#include "EntityAnimal.h"
#include "EntityLookHelper.h"
#include "PathNavigate.h"
#include "World.h"

EntityAIMate::EntityAIMate(EntityAnimal *animal, float speed)
	: theAnimal(animal), theWorld(animal != nullptr ? animal->worldObj : nullptr), targetMateEntityId(-1),
	  spawnBabyDelay(0), moveSpeed(speed)
{
	setMutexBits(3);
}

EntityAnimal *EntityAIMate::resolveMate() const
{
	if (theWorld == nullptr || targetMateEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityAnimal *>(theWorld->getEntityByID(targetMateEntityId));
}

int_t EntityAIMate::findMateEntityId() const
{
	if (theAnimal == nullptr || theWorld == nullptr)
		return -1;

	const std::vector<Entity *> &entities = theWorld->getEntitiesWithinAABB(typeid(*theAnimal),
		theAnimal->boundingBox->expand(8.0, 8.0, 8.0));
	for (Entity *entity : entities)
	{
		EntityAnimal *candidate = dynamic_cast<EntityAnimal *>(entity);
		if (candidate != nullptr && theAnimal->canMateWith(candidate))
			return candidate->entityId;
	}
	return -1;
}

bool EntityAIMate::shouldExecute()
{
	if (theAnimal == nullptr || !theAnimal->isInLove())
		return false;
	targetMateEntityId = findMateEntityId();
	return targetMateEntityId >= 0;
}

bool EntityAIMate::continueExecuting()
{
	EntityAnimal *mate = resolveMate();
	return mate != nullptr && mate->isEntityAlive() && mate->isInLove() && spawnBabyDelay < 60;
}

void EntityAIMate::resetTask()
{
	targetMateEntityId = -1;
	spawnBabyDelay = 0;
}

void EntityAIMate::updateTask()
{
	EntityAnimal *mate = resolveMate();
	if (theAnimal == nullptr || mate == nullptr)
		return;

	if (theAnimal->getLookHelper() != nullptr)
		theAnimal->getLookHelper()->setLookPositionWithEntity(mate, 10.0f, (float)theAnimal->getVerticalFaceSpeed());
	if (theAnimal->getNavigator() != nullptr)
		theAnimal->getNavigator()->tryMoveToEntityLiving(mate, moveSpeed);
	++spawnBabyDelay;
	if (spawnBabyDelay == 60)
		spawnBaby();
}

void EntityAIMate::spawnBaby()
{
	EntityAnimal *mate = resolveMate();
	if (theAnimal == nullptr || mate == nullptr || theWorld == nullptr)
		return;

	EntityAnimal *child = theAnimal->spawnBabyAnimal(mate);
	if (child == nullptr)
		return;

	theAnimal->setGrowingAge(6000);
	mate->setGrowingAge(6000);
	theAnimal->resetInLove();
	mate->resetInLove();
	child->setGrowingAge(-24000);
	child->setLocationAndAngles(theAnimal->posX, theAnimal->posY, theAnimal->posZ, 0.0f, 0.0f);
	if (!theWorld->entityJoinedWorld(child))
		delete child;

	Random &random = theAnimal->getRNG();
	for (int_t i = 0; i < 7; ++i)
	{
		double motionX = random.nextGaussian() * 0.02;
		double motionY = random.nextGaussian() * 0.02;
		double motionZ = random.nextGaussian() * 0.02;
		const double particleX = theAnimal->posX + (double)(random.nextFloat() * theAnimal->width * 2.0f) - (double)theAnimal->width;
		const double particleY = theAnimal->posY + 0.5 + (double)(random.nextFloat() * theAnimal->height);
		const double particleZ = theAnimal->posZ + (double)(random.nextFloat() * theAnimal->width * 2.0f) - (double)theAnimal->width;
		theWorld->spawnParticle("heart", particleX, particleY, particleZ, motionX, motionY, motionZ);
	}
}
