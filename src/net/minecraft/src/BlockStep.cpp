#include "BlockStep.h"

#include "AxisAlignedBB.h"
#include "Block.h"
#include "IBlockAccess.h"
#include "ItemStack.h"
#include "Material.h"
#include "World.h"

const jstring BlockStep::blockStepTypes[6] = { "stone", "sand", "wood", "cobble", "brick", "smoothStoneBrick" };

BlockStep::BlockStep(int_t i, bool flag) : Block(i, 6, Material::rock)
{
	blockType = flag;
	if (!flag)
	{
		setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	}
	setLightOpacity(255);
}

void BlockStep::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	if (blockType)
	{
		setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		return;
	}

	if ((iblockaccess->getBlockMetadata(i, j, k) & 8) != 0)
	{
		setBlockBounds(0.0f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	}
}

void BlockStep::setBlockBoundsForItemRender()
{
	if (blockType)
	{
		setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	}
}

void BlockStep::getCollidingBoundingBoxes(World *world, int_t i, int_t j, int_t k, AxisAlignedBB *axisalignedbb, std::vector<AxisAlignedBB *> &arraylist)
{
	setBlockBoundsBasedOnState(world, i, j, k);
	Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
}

int_t BlockStep::getBlockTextureFromSideAndMetadata(int_t side, int_t metadata)
{
	const int_t type = metadata & 7;
	if (type == 0) return side <= 1 ? 6 : 5;
	if (type == 1) return side == 0 ? 208 : (side == 1 ? 176 : 192);
	if (type == 2) return 4;
	if (type == 3) return 16;
	if (type == 4) return Block::brick->blockIndexInTexture;
	if (type == 5) return Block::stoneBrick->blockIndexInTexture;
	return 6;
}

int_t BlockStep::getBlockTextureFromSide(int_t i)
{
	return getBlockTextureFromSideAndMetadata(i, 0);
}

bool BlockStep::isOpaqueCube()
{
	return blockType;
}

void BlockStep::onBlockPlaced(World *world, int_t i, int_t j, int_t k, int_t side)
{
	if (side == 0 && !blockType)
	{
		const int_t metadata = world->getBlockMetadata(i, j, k) & 7;
		world->setBlockMetadataWithNotify(i, j, k, metadata | 8);
	}
}

int_t BlockStep::idDropped(int_t i, Random &random)
{
	return Block::stairSingle->blockID;
}

int_t BlockStep::quantityDropped(Random &random)
{
	return blockType ? 2 : 1;
}

int_t BlockStep::damageDropped(int_t i)
{
	return i & 7;
}

bool BlockStep::renderAsNormalBlock()
{
	return blockType;
}

bool BlockStep::shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t side)
{
	if (blockType)
	{
		Block::shouldSideBeRendered(iblockaccess, i, j, k, side);
	}

	if (side != 1 && side != 0 && !Block::shouldSideBeRendered(iblockaccess, i, j, k, side))
	{
		return false;
	}

	static const int_t opposite[6] = {1, 0, 3, 2, 5, 4};
	static const int_t offsetX[6] = {0, 0, 0, 0, -1, 1};
	static const int_t offsetY[6] = {-1, 1, 0, 0, 0, 0};
	static const int_t offsetZ[6] = {0, 0, -1, 1, 0, 0};
	const int_t adjacentSide = opposite[side];
	const int_t adjacentX = i + offsetX[adjacentSide];
	const int_t adjacentY = j + offsetY[adjacentSide];
	const int_t adjacentZ = k + offsetZ[adjacentSide];
	const bool adjacentTopHalf = (iblockaccess->getBlockMetadata(adjacentX, adjacentY, adjacentZ) & 8) != 0;

	if (!adjacentTopHalf)
	{
		if (side == 1) return true;
		if (side == 0 && Block::shouldSideBeRendered(iblockaccess, i, j, k, side)) return true;
		return iblockaccess->getBlockId(i, j, k) != blockID || (iblockaccess->getBlockMetadata(i, j, k) & 8) != 0;
	}

	if (side == 0) return true;
	if (side == 1 && Block::shouldSideBeRendered(iblockaccess, i, j, k, side)) return true;
	return iblockaccess->getBlockId(i, j, k) != blockID || (iblockaccess->getBlockMetadata(i, j, k) & 8) == 0;
}

ItemStack *BlockStep::createStackedBlock(int_t metadata)
{
	return new ItemStack(Block::stairSingle, 1, metadata & 7);
}
