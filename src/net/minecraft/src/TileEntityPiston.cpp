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
		// World::removeBlockTileEntity() owns the tile entity outside
		// updateEntities(), and every caller of this method
		// (BlockPistonBase::playBlock's retract path,
		// BlockPistonMoving::onBlockRemoval) runs on the click/notify
		// path -- so the removal ends in `delete` of this very object.
		// Java kept using its garbage-collected `this` afterwards; in C++
		// the old invalidate() below became a virtual call through a freed
		// vtable (the rapid lever+piston toggling prefetch-abort crash on
		// 3DS). Snapshot everything the tail needs, retire the invalid flag
		// while `this` is still alive, and never touch `this` after the
		// removal.
		World *world = worldObj;
		int_t x = xCoord;
		int_t y = yCoord;
		int_t z = zCoord;
		int_t storedId = storedBlockID;
		int_t storedData = storedMetadata;
		lastProgress = progress = 1.0f;
		invalidate();
		world->removeBlockTileEntity(x, y, z);
		if (world->getBlockId(x, y, z) == Block::pistonMoving->blockID)
			world->setBlockAndMetadataWithNotify(x, y, z, storedId, storedData);
	}
}

void TileEntityPiston::updateEntity()
{
	lastProgress = progress;
	if (lastProgress >= 1.0f)
	{
		// World::updateEntities() owns the delete for tile entities retiring
		// themselves mid-pass, so removeBlockTileEntity() only invalidates
		// here -- but keep the same no-`this`-after-removal discipline as
		// clearPistonTileEntity() so this can never decay into the owning
		// delete use-after-free.
		World *world = worldObj;
		int_t x = xCoord;
		int_t y = yCoord;
		int_t z = zCoord;
		int_t storedId = storedBlockID;
		int_t storedData = storedMetadata;
		pushEntities(1.0f, 0.25f);
		invalidate();
		world->removeBlockTileEntity(x, y, z);
		if (world->getBlockId(x, y, z) == Block::pistonMoving->blockID)
			world->setBlockAndMetadataWithNotify(x, y, z, storedId, storedData);
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
