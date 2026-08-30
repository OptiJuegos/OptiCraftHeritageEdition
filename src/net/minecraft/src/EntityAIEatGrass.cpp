#include "EntityAIEatGrass.h"

#include <algorithm>

#include "Block.h"
#include "BlockGrass.h"
#include "BlockTallGrass.h"
#include "EntityLiving.h"
#include "MathHelper.h"
#include "PathNavigate.h"
#include "World.h"

EntityAIEatGrass::EntityAIEatGrass(EntityLiving *entity)
	: theEntity(entity), theWorld(entity != nullptr ? entity->worldObj : nullptr), eatGrassTick(0)
{
	setMutexBits(7);
}

bool EntityAIEatGrass::shouldExecute()
{
	if (theEntity == nullptr || theWorld == nullptr)
		return false;
	if (theEntity->getRNG().nextInt(theEntity->isChild() ? 50 : 1000) != 0)
		return false;
	int_t x = MathHelper::floor_double(theEntity->posX);
	int_t y = MathHelper::floor_double(theEntity->posY);
	int_t z = MathHelper::floor_double(theEntity->posZ);
	if (Block::tallGrass != nullptr && theWorld->getBlockId(x, y, z) == Block::tallGrass->blockID && theWorld->getBlockMetadata(x, y, z) == 1)
		return true;
	return Block::grass != nullptr && theWorld->getBlockId(x, y - 1, z) == Block::grass->blockID;
}

void EntityAIEatGrass::startExecuting()
{
	eatGrassTick = 40;
	if (theWorld != nullptr && theEntity != nullptr)
		theWorld->setEntityState(theEntity, (byte_t)10);
	if (theEntity != nullptr && theEntity->getNavigator() != nullptr)
		theEntity->getNavigator()->clearPathEntity();
}

void EntityAIEatGrass::resetTask()
{
	eatGrassTick = 0;
}

bool EntityAIEatGrass::continueExecuting()
{
	return eatGrassTick > 0;
}

int_t EntityAIEatGrass::getEatGrassTick() const
{
	return eatGrassTick;
}

int_t EntityAIEatGrass::func_48396_h() const
{
	return getEatGrassTick();
}

void EntityAIEatGrass::updateTask()
{
	if (theEntity == nullptr || theWorld == nullptr)
		return;
	eatGrassTick = std::max(0, eatGrassTick - 1);
	if (eatGrassTick != 4)
		return;
	int_t x = MathHelper::floor_double(theEntity->posX);
	int_t y = MathHelper::floor_double(theEntity->posY);
	int_t z = MathHelper::floor_double(theEntity->posZ);
	if (Block::tallGrass != nullptr && theWorld->getBlockId(x, y, z) == Block::tallGrass->blockID)
	{
		theWorld->playAuxSFX(2001, x, y, z, Block::tallGrass->blockID + 4096);
		theWorld->setBlockWithNotify(x, y, z, 0);
		theEntity->eatGrassBonus();
	}
	else if (Block::grass != nullptr && Block::dirt != nullptr && theWorld->getBlockId(x, y - 1, z) == Block::grass->blockID)
	{
		theWorld->playAuxSFX(2001, x, y - 1, z, Block::grass->blockID);
		theWorld->setBlockWithNotify(x, y - 1, z, Block::dirt->blockID);
		theEntity->eatGrassBonus();
	}
}
