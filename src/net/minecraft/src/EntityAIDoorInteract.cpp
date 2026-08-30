#include "EntityAIDoorInteract.h"

#include <algorithm>

#include "Block.h"
#include "BlockDoor.h"
#include "EntityLiving.h"
#include "MathHelper.h"
#include "PathEntity.h"
#include "PathNavigate.h"
#include "PathPoint.h"
#include "World.h"

EntityAIDoorInteract::EntityAIDoorInteract(EntityLiving *entity)
	: theEntity(entity), entityPosX(0), entityPosY(0), entityPosZ(0), targetDoor(nullptr),
	  hasStoppedDoorInteraction(false), entityPositionX(0.0f), entityPositionZ(0.0f)
{
}

bool EntityAIDoorInteract::shouldExecute()
{
	if (theEntity == nullptr || !theEntity->isCollidedHorizontally || theEntity->getNavigator() == nullptr)
		return false;
	PathNavigate *navigator = theEntity->getNavigator();
	PathEntity *path = navigator->getPath();
	if (path == nullptr || path->isFinished() || !navigator->getCanBreakDoors())
		return false;
	int_t limit = std::min(path->getCurrentPathIndex() + 2, path->getCurrentPathLength());
	for (int_t i = 0; i < limit; ++i)
	{
		PathPoint *point = path->getPathPointFromIndex(i);
		if (point == nullptr)
			continue;
		entityPosX = point->xCoord;
		entityPosY = point->yCoord + 1;
		entityPosZ = point->zCoord;
		if (theEntity->getDistanceSq((double)entityPosX, theEntity->posY, (double)entityPosZ) <= 2.25)
		{
			targetDoor = getDoorBlock(entityPosX, entityPosY, entityPosZ);
			if (targetDoor != nullptr)
				return true;
		}
	}
	entityPosX = MathHelper::floor_double(theEntity->posX);
	entityPosY = MathHelper::floor_double(theEntity->posY + 1.0);
	entityPosZ = MathHelper::floor_double(theEntity->posZ);
	targetDoor = getDoorBlock(entityPosX, entityPosY, entityPosZ);
	return targetDoor != nullptr;
}

bool EntityAIDoorInteract::continueExecuting()
{
	return !hasStoppedDoorInteraction;
}

void EntityAIDoorInteract::startExecuting()
{
	hasStoppedDoorInteraction = false;
	entityPositionX = static_cast<float>(static_cast<double>(static_cast<float>(entityPosX) + 0.5f) - theEntity->posX);
	entityPositionZ = static_cast<float>(static_cast<double>(static_cast<float>(entityPosZ) + 0.5f) - theEntity->posZ);
}

void EntityAIDoorInteract::updateTask()
{
	float currentX = static_cast<float>(static_cast<double>(static_cast<float>(entityPosX) + 0.5f) - theEntity->posX);
	float currentZ = static_cast<float>(static_cast<double>(static_cast<float>(entityPosZ) + 0.5f) - theEntity->posZ);
	if (entityPositionX * currentX + entityPositionZ * currentZ < 0.0f)
		hasStoppedDoorInteraction = true;
}

BlockDoor *EntityAIDoorInteract::getDoorBlock(int_t x, int_t y, int_t z) const
{
	if (theEntity == nullptr || theEntity->worldObj == nullptr || Block::doorWood == nullptr)
		return nullptr;
	int_t blockId = theEntity->worldObj->getBlockId(x, y, z);
	if (blockId != Block::doorWood->blockID)
		return nullptr;
	return dynamic_cast<BlockDoor *>(Block::blocksList[blockId]);
}
