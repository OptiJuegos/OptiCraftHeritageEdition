#include "BlockSign.h"
#include "Material.h"
#include "IBlockAccess.h"
#include "World.h"
#include "AxisAlignedBB.h"
#include "Item.h"
#include "TileEntity.h"

BlockSign::BlockSign(int_t i, TileEntity* (*entityFactory)(), bool flag) : BlockContainer(i, Material::wood)
{
	isFreestanding = flag;
	blockIndexInTexture = 4;
	signEntityFactory = entityFactory;
	float f  = 0.25f;
	float f1 = 1.0f;
	setBlockBounds(0.5f - f, 0.0f, 0.5f - f, 0.5f + f, f1, 0.5f + f);
}

AxisAlignedBB *BlockSign::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	return nullptr;
}

AxisAlignedBB *BlockSign::getSelectedBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	setBlockBoundsBasedOnState(world, i, j, k);
	return BlockContainer::getSelectedBoundingBoxFromPool(world, i, j, k);
}

void BlockSign::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	if (isFreestanding) return;
	int_t l  = iblockaccess->getBlockMetadata(i, j, k);
	float f  = 0.28125f;
	float f1 = 0.78125f;
	float f2 = 0.0f;
	float f3 = 1.0f;
	float f4 = 0.125f;
	setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	if (l == 2) setBlockBounds(f2, f, 1.0f - f4, f3, f1, 1.0f);
	if (l == 3) setBlockBounds(f2, f, 0.0f,      f3, f1, f4);
	if (l == 4) setBlockBounds(1.0f - f4, f, f2, 1.0f, f1, f3);
	if (l == 5) setBlockBounds(0.0f, f, f2, f4, f1, f3);
}

int_t BlockSign::getRenderType()       { return -1; }
bool  BlockSign::renderAsNormalBlock() { return false; }
bool  BlockSign::isOpaqueCube()        { return false; }

TileEntity *BlockSign::getBlockEntity()
{
	return signEntityFactory();
}

int_t BlockSign::idDropped(int_t i, Random &random)
{
	return Item::sign->shiftedIndex;
}

void BlockSign::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	bool flag = false;
	if (isFreestanding)
	{
		if (!world->getBlockMaterial(i, j - 1, k)->isSolid()) flag = true;
	}
	else
	{
		int_t i1 = world->getBlockMetadata(i, j, k);
		flag = true;
		if (i1 == 2 && world->getBlockMaterial(i, j, k + 1)->isSolid()) flag = false;
		if (i1 == 3 && world->getBlockMaterial(i, j, k - 1)->isSolid()) flag = false;
		if (i1 == 4 && world->getBlockMaterial(i + 1, j, k)->isSolid()) flag = false;
		if (i1 == 5 && world->getBlockMaterial(i - 1, j, k)->isSolid()) flag = false;
	}
	if (flag)
	{
		dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
		world->setBlockWithNotify(i, j, k, 0);
	}
	BlockContainer::onNeighborBlockChange(world, i, j, k, l);
}

bool BlockSign::getBlocksMovement(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	(void)iblockaccess;
	(void)i;
	(void)j;
	(void)k;
	return true;
}
