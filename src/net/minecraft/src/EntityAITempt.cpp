#include "EntityAITempt.h"

#include <cmath>

#include "EntityCreature.h"
#include "EntityLookHelper.h"
#include "EntityPlayer.h"
#include "ItemStack.h"
#include "PathNavigate.h"
#include "World.h"

EntityAITempt::EntityAITempt(EntityCreature *entity, float speed, int_t itemId, bool scared)
	: temptedEntity(entity), moveSpeed(speed), targetX(0.0), targetY(0.0), targetZ(0.0), targetPitch(0.0), targetYaw(0.0),
	  temptingPlayerEntityId(-1), delayTemptCounter(0), running(false), breedingFood(itemId),
	  scaredByPlayerMovement(scared), oldAvoidsWater(false)
{
	setMutexBits(3);
}

EntityPlayer *EntityAITempt::resolvePlayer() const
{
	if (temptedEntity == nullptr || temptedEntity->worldObj == nullptr || temptingPlayerEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityPlayer *>(temptedEntity->worldObj->getEntityByID(temptingPlayerEntityId));
}

bool EntityAITempt::playerHoldsTemptItem(EntityPlayer *player) const
{
	ItemStack *itemstack = player != nullptr ? player->getCurrentEquippedItem() : nullptr;
	return itemstack != nullptr && itemstack->itemID == breedingFood;
}

bool EntityAITempt::shouldExecute()
{
	if (temptedEntity == nullptr || temptedEntity->worldObj == nullptr)
		return false;
	if (delayTemptCounter > 0)
	{
		--delayTemptCounter;
		return false;
	}

	EntityPlayer *player = temptedEntity->worldObj->getClosestPlayerToEntity(temptedEntity, 10.0);
	if (!playerHoldsTemptItem(player))
	{
		temptingPlayerEntityId = -1;
		return false;
	}
	temptingPlayerEntityId = player->entityId;
	return true;
}

bool EntityAITempt::continueExecuting()
{
	EntityPlayer *player = resolvePlayer();
	if (temptedEntity == nullptr || player == nullptr || !player->isEntityAlive())
		return false;

	if (scaredByPlayerMovement)
	{
		if (temptedEntity->getDistanceSqToEntity(player) < 36.0)
		{
			if (player->getDistanceSq(targetX, targetY, targetZ) > 0.01)
				return false;
			const tempt_math_t pitchDelta = static_cast<tempt_math_t>(player->rotationPitch) - targetPitch;
			const tempt_math_t yawDelta = static_cast<tempt_math_t>(player->rotationYaw) - targetYaw;
			if (std::fabs(pitchDelta) > static_cast<tempt_math_t>(5.0) ||
				std::fabs(yawDelta) > static_cast<tempt_math_t>(5.0))
				return false;
		}
		else
		{
			targetX = player->posX;
			targetY = player->posY;
			targetZ = player->posZ;
		}
		targetPitch = static_cast<tempt_math_t>(player->rotationPitch);
		targetYaw = static_cast<tempt_math_t>(player->rotationYaw);
	}

	EntityPlayer *closest = temptedEntity->worldObj->getClosestPlayerToEntity(temptedEntity, 10.0);
	if (!playerHoldsTemptItem(closest))
	{
		temptingPlayerEntityId = -1;
		return false;
	}
	temptingPlayerEntityId = closest->entityId;
	return true;
}

void EntityAITempt::startExecuting()
{
	EntityPlayer *player = resolvePlayer();
	if (temptedEntity == nullptr || player == nullptr)
		return;
	targetX = player->posX;
	targetY = player->posY;
	targetZ = player->posZ;
	running = true;
	if (temptedEntity->getNavigator() != nullptr)
	{
		oldAvoidsWater = temptedEntity->getNavigator()->getAvoidsWater();
		temptedEntity->getNavigator()->setAvoidsWater(false);
	}
}

void EntityAITempt::resetTask()
{
	temptingPlayerEntityId = -1;
	if (temptedEntity != nullptr && temptedEntity->getNavigator() != nullptr)
	{
		temptedEntity->getNavigator()->clearPathEntity();
		temptedEntity->getNavigator()->setAvoidsWater(oldAvoidsWater);
	}
	delayTemptCounter = 100;
	running = false;
}

void EntityAITempt::updateTask()
{
	EntityPlayer *player = resolvePlayer();
	if (temptedEntity == nullptr || player == nullptr)
		return;
	if (temptedEntity->getLookHelper() != nullptr)
		temptedEntity->getLookHelper()->setLookPositionWithEntity(player, 30.0f, (float)temptedEntity->getVerticalFaceSpeed());
	if (temptedEntity->getNavigator() == nullptr)
		return;
	if (temptedEntity->getDistanceSqToEntity(player) < 6.25)
		temptedEntity->getNavigator()->clearPathEntity();
	else
		temptedEntity->getNavigator()->tryMoveToEntityLiving(player, moveSpeed);
}

bool EntityAITempt::isRunning() const
{
	return running;
}

bool EntityAITempt::func_48270_h() const
{
	return isRunning();
}
