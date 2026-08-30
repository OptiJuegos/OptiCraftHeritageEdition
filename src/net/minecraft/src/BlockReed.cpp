#include "BlockReed.h"
#include "Material.h"
#include "World.h"
#include "Block.h"
#include "Item.h"
#include "AxisAlignedBB.h"
#include "BlockGrass.h"

BlockReed::BlockReed(int_t i, int_t j) : Block(i, Material::plants)
{
	blockIndexInTexture = j;
	float f = 0.375f;
	setBlockBounds(0.5f - f, 0.0f, 0.5f - f, 0.5f + f, 1.0f, 0.5f + f);
	setTickOnLoad(true);
}

void BlockReed::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	if (world->isAirBlock(i, j + 1, k))
	{
		int_t l;
		for (l = 1; world->getBlockId(i, j - l, k) == blockID; l++) {}
		if (l < 3)
		{
			int_t i1 = world->getBlockMetadata(i, j, k);
			if (i1 == 15)
			{
				world->setBlockWithNotify(i, j + 1, k, blockID);
				world->setBlockMetadataWithNotify(i, j, k, 0);
			}
			else
			{
				world->setBlockMetadataWithNotify(i, j, k, i1 + 1);
			}
		}
	}
}

bool BlockReed::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
	int_t l = world->getBlockId(i, j - 1, k);
	if (l == blockID) return true;
	if (l != Block::grass->blockID && l != Block::dirt->blockID) return false;
	if (world->getBlockMaterial(i - 1, j - 1, k) == Material::water) return true;
	if (world->getBlockMaterial(i + 1, j - 1, k) == Material::water) return true;
	if (world->getBlockMaterial(i, j - 1, k - 1) == Material::water) return true;
	return world->getBlockMaterial(i, j - 1, k + 1) == Material::water;
}

void BlockReed::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	checkBlockCoordValid(world, i, j, k);
}

void BlockReed::checkBlockCoordValid(World *world, int_t i, int_t j, int_t k)
{
	if (!canBlockStay(world, i, j, k))
	{
		dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
		world->setBlockWithNotify(i, j, k, 0);
	}
}

bool BlockReed::canBlockStay(World *world, int_t i, int_t j, int_t k)
{
	return canPlaceBlockAt(world, i, j, k);
}

AxisAlignedBB *BlockReed::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	return nullptr;
}

int_t BlockReed::idDropped(int_t i, Random &random)
{
	return Item::reed->shiftedIndex;
}

bool  BlockReed::isOpaqueCube()        { return false; }
bool  BlockReed::renderAsNormalBlock() { return false; }
int_t BlockReed::getRenderType()       { return 1; }
