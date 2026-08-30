#include "EntityAIOcelotSit.h"
#include "java/Arithmetic.h"

#include <limits>

#include "Block.h"
#include "BlockBed.h"
#include "EntityAISit.h"
#include "EntityOcelot.h"
#include "PathNavigate.h"
#include "TileEntity.h"
#include "TileEntityChest.h"
#include "World.h"

EntityAIOcelotSit::EntityAIOcelotSit(EntityOcelot *entity, float moveSpeed)
	: ocelot(entity), speed(moveSpeed), sitTicks(0), pathFailures(0), maxSitTicks(0), targetX(0), targetY(0), targetZ(0)
{
	setMutexBits(5);
}

bool EntityAIOcelotSit::shouldExecute()
{
	return ocelot != nullptr && ocelot->isTamed() && !ocelot->isSitting() &&
		ocelot->getRNG().nextDouble() <= static_cast<double>(0.0065f) && findSittingSpot();
}

bool EntityAIOcelotSit::continueExecuting()
{
	return ocelot != nullptr && sitTicks <= maxSitTicks && pathFailures <= 60 &&
		isSittableBlock(ocelot->worldObj, targetX, targetY, targetZ);
}

void EntityAIOcelotSit::startExecuting()
{
	if (ocelot == nullptr)
		return;
	if (ocelot->getNavigator() != nullptr)
		ocelot->getNavigator()->tryMoveToXYZ((double)((float)targetX + 0.5f), (double)(targetY + 1),
			(double)((float)targetZ + 0.5f), speed);
	sitTicks = 0;
	pathFailures = 0;
	maxSitTicks = ocelot->getRNG().nextInt(ocelot->getRNG().nextInt(1200) + 1200) + 1200;
	if (ocelot->getAISit() != nullptr)
		ocelot->getAISit()->setSitting(false);
}

void EntityAIOcelotSit::resetTask()
{
	if (ocelot != nullptr)
		ocelot->setSitting(false);
}

void EntityAIOcelotSit::updateTask()
{
	if (ocelot == nullptr)
		return;
	++sitTicks;
	if (ocelot->getAISit() != nullptr)
		ocelot->getAISit()->setSitting(false);
	if (ocelot->getDistanceSq((double)targetX, (double)(targetY + 1), (double)targetZ) > 1.0)
	{
		ocelot->setSitting(false);
		if (ocelot->getNavigator() != nullptr)
			ocelot->getNavigator()->tryMoveToXYZ((double)((float)targetX + 0.5f), (double)(targetY + 1),
				(double)((float)targetZ + 0.5f), speed);
		++pathFailures;
	}
	else if (!ocelot->isSitting())
	{
		ocelot->setSitting(true);
	}
	else
	{
		--pathFailures;
	}
}

bool EntityAIOcelotSit::findSittingSpot()
{
	if (ocelot == nullptr || ocelot->worldObj == nullptr)
		return false;
	int_t y = JavaArithmetic::doubleToInt(ocelot->posY);
	double closest = (double)std::numeric_limits<int_t>::max();
	for (int_t x = JavaArithmetic::doubleToInt(ocelot->posX) - 8; (double)x < ocelot->posX + 8.0; ++x)
	{
		for (int_t z = JavaArithmetic::doubleToInt(ocelot->posZ) - 8; (double)z < ocelot->posZ + 8.0; ++z)
		{
			if (!isSittableBlock(ocelot->worldObj, x, y, z) || !ocelot->worldObj->isAirBlock(x, y + 1, z))
				continue;
			double distanceSq = ocelot->getDistanceSq((double)x, (double)y, (double)z);
			if (distanceSq < closest)
			{
				targetX = x;
				targetY = y;
				targetZ = z;
				closest = distanceSq;
			}
		}
	}
	return closest < (double)std::numeric_limits<int_t>::max();
}

bool EntityAIOcelotSit::isSittableBlock(World *world, int_t x, int_t y, int_t z) const
{
	if (world == nullptr)
		return false;
	int_t blockId = world->getBlockId(x, y, z);
	int_t metadata = world->getBlockMetadata(x, y, z);
	if (Block::chest != nullptr && blockId == Block::chest->blockID)
	{
		TileEntityChest *chest = dynamic_cast<TileEntityChest *>(world->getBlockTileEntity(x, y, z));
		return chest != nullptr && chest->numUsingPlayers < 1;
	}
	if (Block::stoneOvenActive != nullptr && blockId == Block::stoneOvenActive->blockID)
		return true;
	return Block::blockBed != nullptr && blockId == Block::blockBed->blockID && !BlockBed::isBlockFootOfBed(metadata);
}
