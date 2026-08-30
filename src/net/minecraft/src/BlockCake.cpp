#include "BlockCake.h"
#include "Material.h"
#include "IBlockAccess.h"
#include "World.h"
#include "AxisAlignedBB.h"
#include "EntityPlayer.h"
#include "FoodStats.h"

BlockCake::BlockCake(int_t i, int_t j) : Block(i, j, Material::cakeMaterial)
{
	setTickOnLoad(true);
}

void BlockCake::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	int_t l  = iblockaccess->getBlockMetadata(i, j, k);
	float f  = 0.0625f;
	float f1 = (float)(1 + l * 2) / 16.0f;
	float f2 = 0.5f;
	setBlockBounds(f1, 0.0f, f, 1.0f - f, f2, 1.0f - f);
}

void BlockCake::setBlockBoundsForItemRender()
{
	float f  = 0.0625f;
	float f1 = 0.5f;
	setBlockBounds(f, 0.0f, f, 1.0f - f, f1, 1.0f - f);
}

AxisAlignedBB *BlockCake::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	int_t l  = world->getBlockMetadata(i, j, k);
	float f  = 0.0625f;
	float f1 = (float)(1 + l * 2) / 16.0f;
	float f2 = 0.5f;
	return AxisAlignedBB::getBoundingBoxFromPool((float)i + f1, j, (float)k + f,
	                                             (float)(i + 1) - f, ((float)j + f2) - f, (float)(k + 1) - f);
}

AxisAlignedBB *BlockCake::getSelectedBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	int_t l  = world->getBlockMetadata(i, j, k);
	float f  = 0.0625f;
	float f1 = (float)(1 + l * 2) / 16.0f;
	float f2 = 0.5f;
	return AxisAlignedBB::getBoundingBoxFromPool((float)i + f1, j, (float)k + f,
	                                             (float)(i + 1) - f, (float)j + f2, (float)(k + 1) - f);
}

int_t BlockCake::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
	if (i == 1) return blockIndexInTexture;
	if (i == 0) return blockIndexInTexture + 3;
	if (j > 0 && i == 4) return blockIndexInTexture + 2;
	return blockIndexInTexture + 1;
}

int_t BlockCake::getBlockTextureFromSide(int_t i)
{
	if (i == 1) return blockIndexInTexture;
	if (i == 0) return blockIndexInTexture + 3;
	return blockIndexInTexture + 1;
}

bool BlockCake::renderAsNormalBlock() { return false; }
bool BlockCake::isOpaqueCube()        { return false; }

bool BlockCake::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
	eatCakeSlice(world, i, j, k, entityplayer);
	return true;
}

void BlockCake::onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
	eatCakeSlice(world, i, j, k, entityplayer);
}

void BlockCake::eatCakeSlice(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
	if (entityplayer->canEat(false))
	{
		entityplayer->getFoodStats()->addStats(2, 0.1f);
		int_t l = world->getBlockMetadata(i, j, k) + 1;
		if (l >= 6)
		{
			world->setBlockWithNotify(i, j, k, 0);
		}
		else
		{
			world->setBlockMetadataWithNotify(i, j, k, l);
			world->markBlockAsNeedsUpdate(i, j, k);
		}
	}
}

bool BlockCake::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
	if (!Block::canPlaceBlockAt(world, i, j, k)) return false;
	return canBlockStay(world, i, j, k);
}

void BlockCake::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	if (!canBlockStay(world, i, j, k))
	{
		dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k), 0);
		world->setBlockWithNotify(i, j, k, 0);
	}
}

bool BlockCake::canBlockStay(World *world, int_t i, int_t j, int_t k)
{
	return world->getBlockMaterial(i, j - 1, k)->isSolid();
}

int_t BlockCake::quantityDropped(Random &random) { return 0; }
int_t BlockCake::idDropped(int_t i, Random &random) { return 0; }
