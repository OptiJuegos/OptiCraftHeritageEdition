#include "BlockSnow.h"
#include "Material.h"
#include "World.h"
#include "Block.h"
#include "BlockLeaves.h"
#include "AxisAlignedBB.h"
#include "IBlockAccess.h"
#include "Item.h"
#include "EntityItem.h"
#include "ItemStack.h"
#include "StatList.h"
#include "EntityPlayer.h"
#include "EnumSkyBlock.h"

BlockSnow::BlockSnow(int_t i, int_t j) : Block(i, j, Material::snow)
{
	setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.125f, 1.0f);
	setTickOnLoad(true);
}

AxisAlignedBB *BlockSnow::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	int_t l = world->getBlockMetadata(i, j, k) & 7;
	if (l >= 3)
	{
		return AxisAlignedBB::getBoundingBoxFromPool((double)i + minX, (double)j + minY, (double)k + minZ,
		                                             (double)i + maxX, (float)j + 0.5f, (double)k + maxZ);
	}
	return nullptr;
}

bool BlockSnow::isOpaqueCube()        { return false; }
bool BlockSnow::renderAsNormalBlock() { return false; }

void BlockSnow::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	int_t l = iblockaccess->getBlockMetadata(i, j, k) & 7;
	float f = (float)(2 * (1 + l)) / 16.0f;
	setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, f, 1.0f);
}

bool BlockSnow::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
	int_t l = world->getBlockId(i, j - 1, k);
	if (l == 0 || (l != Block::leaves->blockID && !Block::blocksList[l]->isOpaqueCube())) return false;
	return world->getBlockMaterial(i, j - 1, k)->blocksMovement();
}

void BlockSnow::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	getGlowing(world, i, j, k);
}

bool BlockSnow::getGlowing(World *world, int_t i, int_t j, int_t k)
{
	if (!canPlaceBlockAt(world, i, j, k))
	{
		dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
		world->setBlockWithNotify(i, j, k, 0);
		return false;
	}
	return true;
}

void BlockSnow::harvestBlock(World *world, EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l)
{
	int_t i1 = Item::snowball->shiftedIndex;
	float f = 0.7f;
	double d  = (double)(world->rand.nextFloat() * f) + (double)(1.0f - f) * 0.5;
	double d1 = (double)(world->rand.nextFloat() * f) + (double)(1.0f - f) * 0.5;
	double d2 = (double)(world->rand.nextFloat() * f) + (double)(1.0f - f) * 0.5;
	EntityItem *entityitem = new EntityItem(world, (double)i + d, (double)j + d1, (double)k + d2, new ItemStack(i1, 1, 0));
	entityitem->delayBeforeCanPickup = 10;
	if (!world->entityJoinedWorld(entityitem))
		delete entityitem;
	world->setBlockWithNotify(i, j, k, 0);
	entityplayer->addStat(StatList::mineBlockStatArray[blockID], 1);
}

int_t BlockSnow::idDropped(int_t i, Random &random)
{
	return Item::snowball->shiftedIndex;
}

int_t BlockSnow::quantityDropped(Random &random) { return 0; }

void BlockSnow::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	if (world->getSavedLightValue(EnumSkyBlock::Block, i, j, k) > 11)
	{
		dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
		world->setBlockWithNotify(i, j, k, 0);
	}
}

bool BlockSnow::shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	if (l == 1) return true;
	return Block::shouldSideBeRendered(iblockaccess, i, j, k, l);
}
