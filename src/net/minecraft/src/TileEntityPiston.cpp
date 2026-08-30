#include "TileEntityPiston.h"

#include <vector>

#include "AxisAlignedBB.h"
#include "Block.h"
#include "BlockPistonMoving.h"
#include "Entity.h"
#include "NBTTagCompound.h"
#include "PistonBlockTextures.h"
#include "World.h"

TileEntityPiston::TileEntityPiston()
	: storedBlockID(0),
	  storedMetadata(0),
	  pistonOrientation(0),
	  extending(false),
	  fieldIsHead(false),
	  progress(0.0f),
	  lastProgress(0.0f)
{
}

TileEntityPiston::TileEntityPiston(int_t storedBlockID, int_t storedMetadata, int_t orientation, bool extending, bool isHead)
	: storedBlockID(storedBlockID),
	  storedMetadata(storedMetadata),
	  pistonOrientation(orientation),
	  extending(extending),
	  fieldIsHead(isHead),
	  progress(0.0f),
	  lastProgress(0.0f)
{
}

int_t TileEntityPiston::getStoredBlockID()  { return storedBlockID; }
int_t TileEntityPiston::getBlockMetadata()  { return storedMetadata; }
bool  TileEntityPiston::isExtending()       { return extending; }
int_t TileEntityPiston::getOrientation()    { return pistonOrientation; }
bool  TileEntityPiston::isHead()            { return fieldIsHead; }

float TileEntityPiston::getProgress(float partialTicks)
{
	if (partialTicks > 1.0f)
		partialTicks = 1.0f;
	return lastProgress + (progress - lastProgress) * partialTicks;
}

float TileEntityPiston::getOffsetX(float partialTicks)
{
	if (extending)
		return (getProgress(partialTicks) - 1.0f) * (float)PistonBlockTextures::deltaX[pistonOrientation];
	return (1.0f - getProgress(partialTicks)) * (float)PistonBlockTextures::deltaX[pistonOrientation];
}

float TileEntityPiston::getOffsetY(float partialTicks)
{
	if (extending)
		return (getProgress(partialTicks) - 1.0f) * (float)PistonBlockTextures::deltaY[pistonOrientation];
	return (1.0f - getProgress(partialTicks)) * (float)PistonBlockTextures::deltaY[pistonOrientation];
}

float TileEntityPiston::getOffsetZ(float partialTicks)
{
	if (extending)
		return (getProgress(partialTicks) - 1.0f) * (float)PistonBlockTextures::deltaZ[pistonOrientation];
	return (1.0f - getProgress(partialTicks)) * (float)PistonBlockTextures::deltaZ[pistonOrientation];
}

void TileEntityPiston::pushEntities(float progress, float delta)
{
	if (!extending)
		progress -= 1.0f;
	else
		progress = 1.0f - progress;

	AxisAlignedBB *axisalignedbb = Block::pistonMoving->getMovingBlockCollisionBox(
		worldObj, xCoord, yCoord, zCoord, storedBlockID, progress, pistonOrientation);
	if (axisalignedbb == nullptr)
		return;

	std::vector<Entity *> &list = worldObj->getEntitiesWithinAABBExcludingEntity(nullptr, axisalignedbb);
	if (list.empty())
		return;

	// World reuses the query vector as a scratch buffer. moveEntity() can query
	// collisions again, so mirror Java's pushedObjects snapshot before moving.
	static std::vector<Entity *> pushedObjects;
	pushedObjects.assign(list.begin(), list.end());
	for (Entity *entity : pushedObjects)
	{
		entity->moveEntity(
			delta * (float)PistonBlockTextures::deltaX[pistonOrientation],
			delta * (float)PistonBlockTextures::deltaY[pistonOrientation],
			delta * (float)PistonBlockTextures::deltaZ[pistonOrientation]);
	}
	pushedObjects.clear();
}

void TileEntityPiston::clearPistonTileEntity()
{
	if (lastProgress < 1.0f && worldObj != nullptr)
	{
		lastProgress = progress = 1.0f;
		worldObj->removeBlockTileEntity(xCoord, yCoord, zCoord);
		invalidate();
		if (worldObj->getBlockId(xCoord, yCoord, zCoord) == Block::pistonMoving->blockID)
			worldObj->setBlockAndMetadataWithNotify(xCoord, yCoord, zCoord, storedBlockID, storedMetadata);
	}
}

void TileEntityPiston::updateEntity()
{
	lastProgress = progress;
	if (lastProgress >= 1.0f)
	{
		pushEntities(1.0f, 0.25f);
		worldObj->removeBlockTileEntity(xCoord, yCoord, zCoord);
		invalidate();
		if (worldObj->getBlockId(xCoord, yCoord, zCoord) == Block::pistonMoving->blockID)
			worldObj->setBlockAndMetadataWithNotify(xCoord, yCoord, zCoord, storedBlockID, storedMetadata);
		return;
	}

	progress += 0.5f;
	if (progress >= 1.0f)
		progress = 1.0f;
	if (extending)
		pushEntities(progress, (progress - lastProgress) + 0.0625f);
}

void TileEntityPiston::readFromNBT(NBTTagCompound *nbttagcompound)
{
	TileEntity::readFromNBT(nbttagcompound);
	storedBlockID     = nbttagcompound->getInteger("blockId");
	storedMetadata    = nbttagcompound->getInteger("blockData");
	pistonOrientation = nbttagcompound->getInteger("facing");
	lastProgress = progress = nbttagcompound->getFloat("progress");
	extending = nbttagcompound->getBoolean("extending");
}

void TileEntityPiston::writeToNBT(NBTTagCompound *nbttagcompound)
{
	TileEntity::writeToNBT(nbttagcompound);
	nbttagcompound->setInteger("blockId",   storedBlockID);
	nbttagcompound->setInteger("blockData", storedMetadata);
	nbttagcompound->setInteger("facing",    pistonOrientation);
	nbttagcompound->setFloat  ("progress",  lastProgress);
	nbttagcompound->setBoolean("extending", extending);
}
