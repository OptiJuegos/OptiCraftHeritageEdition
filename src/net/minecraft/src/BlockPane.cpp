#include "BlockPane.h"

#include "IBlockAccess.h"
#include "Material.h"
#include "World.h"

BlockPane::BlockPane(int_t id, int_t texture, int_t sideTexture, Material *material, bool canDrop) :
	Block(id, texture, material),
	sideTextureIndex(sideTexture),
	canDropItself(canDrop)
{
}

int_t BlockPane::idDropped(int_t metadata, Random &random, int_t fortune)
{
	return canDropItself ? Block::idDropped(metadata, random, fortune) : 0;
}

bool BlockPane::isOpaqueCube()
{
	return false;
}

bool BlockPane::renderAsNormalBlock()
{
	return false;
}

int_t BlockPane::getRenderType()
{
	return 18;
}

bool BlockPane::shouldSideBeRendered(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z, int_t side)
{
	return iblockaccess->getBlockId(x, y, z) != blockID && Block::shouldSideBeRendered(iblockaccess, x, y, z, side);
}

void BlockPane::getCollidingBoundingBoxes(World *world, int_t x, int_t y, int_t z, AxisAlignedBB *mask, std::vector<AxisAlignedBB *> &list)
{
	const bool north = canThisPaneConnectToThisBlockID(world->getBlockId(x, y, z - 1));
	const bool south = canThisPaneConnectToThisBlockID(world->getBlockId(x, y, z + 1));
	const bool west = canThisPaneConnectToThisBlockID(world->getBlockId(x - 1, y, z));
	const bool east = canThisPaneConnectToThisBlockID(world->getBlockId(x + 1, y, z));

	if ((!west || !east) && (west || east || north || south))
	{
		if (west && !east)
		{
			setBlockBounds(0.0f, 0.0f, 7.0f / 16.0f, 0.5f, 1.0f, 9.0f / 16.0f);
			Block::getCollidingBoundingBoxes(world, x, y, z, mask, list);
		}
		else if (!west && east)
		{
			setBlockBounds(0.5f, 0.0f, 7.0f / 16.0f, 1.0f, 1.0f, 9.0f / 16.0f);
			Block::getCollidingBoundingBoxes(world, x, y, z, mask, list);
		}
	}
	else
	{
		setBlockBounds(0.0f, 0.0f, 7.0f / 16.0f, 1.0f, 1.0f, 9.0f / 16.0f);
		Block::getCollidingBoundingBoxes(world, x, y, z, mask, list);
	}

	if ((!north || !south) && (west || east || north || south))
	{
		if (north && !south)
		{
			setBlockBounds(7.0f / 16.0f, 0.0f, 0.0f, 9.0f / 16.0f, 1.0f, 0.5f);
			Block::getCollidingBoundingBoxes(world, x, y, z, mask, list);
		}
		else if (!north && south)
		{
			setBlockBounds(7.0f / 16.0f, 0.0f, 0.5f, 9.0f / 16.0f, 1.0f, 1.0f);
			Block::getCollidingBoundingBoxes(world, x, y, z, mask, list);
		}
	}
	else
	{
		setBlockBounds(7.0f / 16.0f, 0.0f, 0.0f, 9.0f / 16.0f, 1.0f, 1.0f);
		Block::getCollidingBoundingBoxes(world, x, y, z, mask, list);
	}
}

void BlockPane::setBlockBoundsForItemRender()
{
	setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

void BlockPane::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z)
{
	float minX = 7.0f / 16.0f;
	float maxX = 9.0f / 16.0f;
	float minZ = 7.0f / 16.0f;
	float maxZ = 9.0f / 16.0f;
	const bool north = canThisPaneConnectToThisBlockID(iblockaccess->getBlockId(x, y, z - 1));
	const bool south = canThisPaneConnectToThisBlockID(iblockaccess->getBlockId(x, y, z + 1));
	const bool west = canThisPaneConnectToThisBlockID(iblockaccess->getBlockId(x - 1, y, z));
	const bool east = canThisPaneConnectToThisBlockID(iblockaccess->getBlockId(x + 1, y, z));

	if ((!west || !east) && (west || east || north || south))
	{
		if (west && !east) minX = 0.0f;
		else if (!west && east) maxX = 1.0f;
	}
	else
	{
		minX = 0.0f;
		maxX = 1.0f;
	}

	if ((!north || !south) && (west || east || north || south))
	{
		if (north && !south) minZ = 0.0f;
		else if (!north && south) maxZ = 1.0f;
	}
	else
	{
		minZ = 0.0f;
		maxZ = 1.0f;
	}

	setBlockBounds(minX, 0.0f, minZ, maxX, 1.0f, maxZ);
}

int_t BlockPane::getSideTextureIndex() const
{
	return sideTextureIndex;
}

bool BlockPane::canThisPaneConnectToThisBlockID(int_t id) const
{
	if (id < 0 || id >= BLOCK_REGISTRY_SIZE)
	{
		return false;
	}
	return opaqueCubeLookup[id] || id == blockID || (glass != nullptr && id == glass->blockID);
}
