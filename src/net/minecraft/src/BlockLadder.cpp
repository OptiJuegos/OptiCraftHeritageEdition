#include "BlockLadder.h"
#include "Material.h"
#include "World.h"
#include "AxisAlignedBB.h"
#include "Block.h"

BlockLadder::BlockLadder(int_t i, int_t j) : Block(i, j, Material::circuits)
{
}

AxisAlignedBB *BlockLadder::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	int_t l = world->getBlockMetadata(i, j, k);
	float f = 0.125f;
	if (l == 2) setBlockBounds(0.0f, 0.0f, 1.0f - f, 1.0f, 1.0f, 1.0f);
	if (l == 3) setBlockBounds(0.0f, 0.0f, 0.0f,     1.0f, 1.0f, f);
	if (l == 4) setBlockBounds(1.0f - f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	if (l == 5) setBlockBounds(0.0f, 0.0f, 0.0f,     f, 1.0f, 1.0f);
	return Block::getCollisionBoundingBoxFromPool(world, i, j, k);
}

AxisAlignedBB *BlockLadder::getSelectedBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	int_t l = world->getBlockMetadata(i, j, k);
	float f = 0.125f;
	if (l == 2) setBlockBounds(0.0f, 0.0f, 1.0f - f, 1.0f, 1.0f, 1.0f);
	if (l == 3) setBlockBounds(0.0f, 0.0f, 0.0f,     1.0f, 1.0f, f);
	if (l == 4) setBlockBounds(1.0f - f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	if (l == 5) setBlockBounds(0.0f, 0.0f, 0.0f,     f, 1.0f, 1.0f);
	return Block::getSelectedBoundingBoxFromPool(world, i, j, k);
}

bool  BlockLadder::isOpaqueCube()        { return false; }
bool  BlockLadder::renderAsNormalBlock() { return false; }
int_t BlockLadder::getRenderType()       { return 8; }

bool BlockLadder::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
	if (world->isBlockNormalCube(i - 1, j, k)) return true;
	if (world->isBlockNormalCube(i + 1, j, k)) return true;
	if (world->isBlockNormalCube(i, j, k - 1)) return true;
	return world->isBlockNormalCube(i, j, k + 1);
}

void BlockLadder::onBlockPlaced(World *world, int_t i, int_t j, int_t k, int_t l)
{
	int_t i1 = world->getBlockMetadata(i, j, k);
	if ((i1 == 0 || l == 2) && world->isBlockNormalCube(i, j, k + 1)) i1 = 2;
	if ((i1 == 0 || l == 3) && world->isBlockNormalCube(i, j, k - 1)) i1 = 3;
	if ((i1 == 0 || l == 4) && world->isBlockNormalCube(i + 1, j, k)) i1 = 4;
	if ((i1 == 0 || l == 5) && world->isBlockNormalCube(i - 1, j, k)) i1 = 5;
	world->setBlockMetadataWithNotify(i, j, k, i1);
}

void BlockLadder::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	int_t i1 = world->getBlockMetadata(i, j, k);
	bool flag = false;
	if (i1 == 2 && world->isBlockNormalCube(i, j, k + 1)) flag = true;
	if (i1 == 3 && world->isBlockNormalCube(i, j, k - 1)) flag = true;
	if (i1 == 4 && world->isBlockNormalCube(i + 1, j, k)) flag = true;
	if (i1 == 5 && world->isBlockNormalCube(i - 1, j, k)) flag = true;
	if (!flag)
	{
		dropBlockAsItem(world, i, j, k, i1);
		world->setBlockWithNotify(i, j, k, 0);
	}
	Block::onNeighborBlockChange(world, i, j, k, l);
}

int_t BlockLadder::quantityDropped(Random &random) { return 1; }
