#include "BlockPistonMoving.h"

#include "AxisAlignedBB.h"
#include "Block.h"
#include "EntityPlayer.h"
#include "IBlockAccess.h"
#include "Material.h"
#include "PistonBlockTextures.h"
#include "TileEntity.h"
#include "TileEntityPiston.h"
#include "World.h"

BlockPistonMoving::BlockPistonMoving(int_t i)
	: BlockContainer(i, Material::piston)
{
	setHardness(-1.0f);
}

TileEntity *BlockPistonMoving::getBlockEntity()
{
	return nullptr;
}

void BlockPistonMoving::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
}

void BlockPistonMoving::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
	TileEntity *tileentity = world->getBlockTileEntity(i, j, k);
	if (TileEntityPiston *piston = dynamic_cast<TileEntityPiston *>(tileentity))
		piston->clearPistonTileEntity();
	else
		BlockContainer::onBlockRemoval(world, i, j, k);
}

bool BlockPistonMoving::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
	return false;
}

bool BlockPistonMoving::canPlaceBlockOnSide(World *world, int_t i, int_t j, int_t k, int_t l)
{
	return false;
}

int_t BlockPistonMoving::getRenderType()       { return -1; }
bool  BlockPistonMoving::isOpaqueCube()        { return false; }
bool  BlockPistonMoving::renderAsNormalBlock() { return false; }

bool BlockPistonMoving::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
	if (!world->multiplayerWorld && world->getBlockTileEntity(i, j, k) == nullptr)
	{
		world->setBlockWithNotify(i, j, k, 0);
		return true;
	}
	return false;
}

int_t BlockPistonMoving::idDropped(int_t i, Random &random)
{
	return 0;
}

void BlockPistonMoving::dropBlockAsItemWithChance(World *world, int_t i, int_t j, int_t k, int_t l, float f)
{
	if (world->multiplayerWorld)
		return;
	TileEntityPiston *piston = getPistonTileEntityAt(world, i, j, k);
	if (piston == nullptr)
		return;
	Block::blocksList[piston->getStoredBlockID()]->dropBlockAsItem(
		world, i, j, k, piston->getBlockMetadata());
}

void BlockPistonMoving::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	if (world->multiplayerWorld)
		return;
	// Java intentionally evaluates the call and discards it (`if(...);`).
	(void)world->getBlockTileEntity(i, j, k);
}

TileEntity *BlockPistonMoving::createMovingPistonTileEntity(int_t i, int_t j, int_t k, bool extending, bool isHead)
{
	return new TileEntityPiston(i, j, k, extending, isHead);
}

TileEntity *BlockPistonMoving::getTileEntity(int_t blockId, int_t metadata, int_t orientation, bool extending, bool isHead)
{
	return createMovingPistonTileEntity(blockId, metadata, orientation, extending, isHead);
}


AxisAlignedBB *BlockPistonMoving::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	TileEntityPiston *piston = getPistonTileEntityAt(world, i, j, k);
	if (piston == nullptr)
		return nullptr;
	float f = piston->getProgress(0.0f);
	if (piston->isExtending())
		f = 1.0f - f;
	return getMovingBlockCollisionBox(world, i, j, k, piston->getStoredBlockID(), f, piston->getOrientation());
}

void BlockPistonMoving::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	TileEntityPiston *piston = getPistonTileEntityAt(iblockaccess, i, j, k);
	if (piston == nullptr)
		return;

	Block *block = Block::blocksList[piston->getStoredBlockID()];
	if (block == nullptr || block == this)
		return;

	block->setBlockBoundsBasedOnState(iblockaccess, i, j, k);
	float f = piston->getProgress(0.0f);
	if (piston->isExtending())
		f = 1.0f - f;

	int_t orientation = piston->getOrientation();
	minX = block->minX - (double)((float)PistonBlockTextures::deltaX[orientation] * f);
	minY = block->minY - (double)((float)PistonBlockTextures::deltaY[orientation] * f);
	minZ = block->minZ - (double)((float)PistonBlockTextures::deltaZ[orientation] * f);
	maxX = block->maxX - (double)((float)PistonBlockTextures::deltaX[orientation] * f);
	maxY = block->maxY - (double)((float)PistonBlockTextures::deltaY[orientation] * f);
	maxZ = block->maxZ - (double)((float)PistonBlockTextures::deltaZ[orientation] * f);
}

AxisAlignedBB *BlockPistonMoving::getMovingBlockCollisionBox(World *world, int_t i, int_t j, int_t k, int_t blockId, float progress, int_t orientation)
{
	if (blockId == 0 || blockId == blockID)
		return nullptr;
	AxisAlignedBB *axisalignedbb = Block::blocksList[blockId]->getCollisionBoundingBoxFromPool(world, i, j, k);
	if (axisalignedbb == nullptr)
		return nullptr;

	if (PistonBlockTextures::deltaX[orientation] < 0)
		axisalignedbb->minX -= (float)PistonBlockTextures::deltaX[orientation] * progress;
	else
		axisalignedbb->maxX -= (float)PistonBlockTextures::deltaX[orientation] * progress;

	if (PistonBlockTextures::deltaY[orientation] < 0)
		axisalignedbb->minY -= (float)PistonBlockTextures::deltaY[orientation] * progress;
	else
		axisalignedbb->maxY -= (float)PistonBlockTextures::deltaY[orientation] * progress;

	if (PistonBlockTextures::deltaZ[orientation] < 0)
		axisalignedbb->minZ -= (float)PistonBlockTextures::deltaZ[orientation] * progress;
	else
		axisalignedbb->maxZ -= (float)PistonBlockTextures::deltaZ[orientation] * progress;

	return axisalignedbb;
}

TileEntityPiston *BlockPistonMoving::getPistonTileEntityAt(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	TileEntity *tileentity = iblockaccess->getBlockTileEntity(i, j, k);
	return dynamic_cast<TileEntityPiston *>(tileentity);
}
