#include "EntityAIBeg.h"

#include "EntityLookHelper.h"
#include "EntityPlayer.h"
#include "EntityWolf.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "World.h"

EntityAIBeg::EntityAIBeg(EntityWolf *wolf, float distance)
	: wolf(wolf), playerEntityId(-1), maxDistance(distance), lookTime(0)
{
	setMutexBits(2);
}

EntityPlayer *EntityAIBeg::resolvePlayer() const
{
	if (wolf == nullptr || wolf->worldObj == nullptr || playerEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityPlayer *>(wolf->worldObj->getEntityByID(playerEntityId));
}

bool EntityAIBeg::playerHasInterestingItem(EntityPlayer *player) const
{
	if (wolf == nullptr || player == nullptr)
		return false;
	ItemStack *stack = player->inventory != nullptr ? player->inventory->getCurrentItem() : nullptr;
	if (stack == nullptr)
		return false;
	if (!wolf->isTamed() && Item::bone != nullptr && stack->itemID == Item::bone->shiftedIndex)
		return true;
	return wolf->isWheat(stack);
}

bool EntityAIBeg::shouldExecute()
{
	playerEntityId = -1;
	if (wolf == nullptr || wolf->worldObj == nullptr)
		return false;
	EntityPlayer *player = wolf->worldObj->getClosestPlayerToEntity(wolf, (double)maxDistance);
	if (!playerHasInterestingItem(player))
		return false;
	playerEntityId = player->entityId;
	return true;
}

bool EntityAIBeg::continueExecuting()
{
	EntityPlayer *player = resolvePlayer();
	return player != nullptr && player->isEntityAlive() && wolf != nullptr &&
		wolf->getDistanceSqToEntity(player) <= (double)(maxDistance * maxDistance) && lookTime > 0 &&
		playerHasInterestingItem(player);
}

void EntityAIBeg::startExecuting()
{
	if (wolf == nullptr)
		return;
	wolf->setLooksWithInterest(true);
	lookTime = 40 + wolf->getRNG().nextInt(40);
}

void EntityAIBeg::resetTask()
{
	if (wolf != nullptr)
		wolf->setLooksWithInterest(false);
	playerEntityId = -1;
}

void EntityAIBeg::updateTask()
{
	EntityPlayer *player = resolvePlayer();
	if (wolf == nullptr || player == nullptr || wolf->getLookHelper() == nullptr)
		return;
	wolf->getLookHelper()->setLookPosition(player->posX, player->posY + (double)player->getEyeHeight(), player->posZ,
		10.0f, (float)wolf->getVerticalFaceSpeed());
	--lookTime;
}
