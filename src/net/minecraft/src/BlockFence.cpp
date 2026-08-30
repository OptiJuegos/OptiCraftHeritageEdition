#include "BlockFence.h"

#include "AxisAlignedBB.h"
#include "IBlockAccess.h"
#include "Material.h"
#include "World.h"

BlockFence::BlockFence(int_t id, int_t texture) :
	Block(id, texture, Material::wood)
{
}

BlockFence::BlockFence(int_t id, int_t texture, Material *material) :
	Block(id, texture, material)
{
}

bool BlockFence::canPlaceBlockAt(World *world, int_t x, int_t y, int_t z)
{
	return Block::canPlaceBlockAt(world, x, y, z);
}

AxisAlignedBB *BlockFence::getCollisionBoundingBoxFromPool(World *world, int_t x, int_t y, int_t z)
{
	const bool north = canConnectFenceTo(world, x, y, z - 1);
	const bool south = canConnectFenceTo(world, x, y, z + 1);
	const bool west = canConnectFenceTo(world, x - 1, y, z);
	const bool east = canConnectFenceTo(world, x + 1, y, z);
	float minX = 6.0f / 16.0f;
	float maxX = 10.0f / 16.0f;
	float minZ = 6.0f / 16.0f;
	float maxZ = 10.0f / 16.0f;
	if (north) minZ = 0.0f;
	if (south) maxZ = 1.0f;
	if (west) minX = 0.0f;
	if (east) maxX = 1.0f;
	return AxisAlignedBB::getBoundingBoxFromPool((float)x + minX, y, (float)z + minZ,
		(float)x + maxX, (float)y + 1.5f, (float)z + maxZ);
}

void BlockFence::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z)
{
	const bool north = canConnectFenceTo(iblockaccess, x, y, z - 1);
	const bool south = canConnectFenceTo(iblockaccess, x, y, z + 1);
	const bool west = canConnectFenceTo(iblockaccess, x - 1, y, z);
	const bool east = canConnectFenceTo(iblockaccess, x + 1, y, z);
	float minX = 6.0f / 16.0f;
	float maxX = 10.0f / 16.0f;
	float minZ = 6.0f / 16.0f;
	float maxZ = 10.0f / 16.0f;
	if (north) minZ = 0.0f;
	if (south) maxZ = 1.0f;
	if (west) minX = 0.0f;
	if (east) maxX = 1.0f;
	setBlockBounds(minX, 0.0f, minZ, maxX, 1.0f, maxZ);
}

bool BlockFence::isOpaqueCube()
{
	return false;
}

bool BlockFence::renderAsNormalBlock()
{
	return false;
}

bool BlockFence::getBlocksMovement(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z)
{
	return false;
}

int_t BlockFence::getRenderType()
{
	return 11;
}

bool BlockFence::canConnectFenceTo(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z)
{
	const int_t id = iblockaccess->getBlockId(x, y, z);
	if (id == blockID || (fenceGate != nullptr && id == fenceGate->blockID))
	{
		return true;
	}
	if (id <= 0 || id >= BLOCK_REGISTRY_SIZE)
	{
		return false;
	}
	Block *block = blocksList[id];
	return block != nullptr && block->blockMaterial->getIsTranslucent() && block->renderAsNormalBlock() && block->blockMaterial != Material::pumpkin;
}
