#include "BlockFenceGate.h"

#include "AxisAlignedBB.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "IBlockAccess.h"
#include "Material.h"
#include "MathHelper.h"
#include "World.h"

BlockFenceGate::BlockFenceGate(int_t id, int_t texture) :
	BlockDirectional(id, texture, Material::wood)
{
}

bool BlockFenceGate::canPlaceBlockAt(World *world, int_t x, int_t y, int_t z)
{
	return world->getBlockMaterial(x, y - 1, z)->isSolid() && Block::canPlaceBlockAt(world, x, y, z);
}

AxisAlignedBB *BlockFenceGate::getCollisionBoundingBoxFromPool(World *world, int_t x, int_t y, int_t z)
{
	const int_t metadata = world->getBlockMetadata(x, y, z);
	if (isFenceGateOpen(metadata))
	{
		return nullptr;
	}
	if (metadata != 2 && metadata != 0)
	{
		return AxisAlignedBB::getBoundingBoxFromPool((float)x + 6.0f / 16.0f, y, z,
			(float)x + 10.0f / 16.0f, (float)y + 1.5f, z + 1);
	}
	return AxisAlignedBB::getBoundingBoxFromPool(x, y, (float)z + 6.0f / 16.0f,
		x + 1, (float)y + 1.5f, (float)z + 10.0f / 16.0f);
}

void BlockFenceGate::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z)
{
	const int_t direction = getDirection(iblockaccess->getBlockMetadata(x, y, z));
	if (direction != 2 && direction != 0)
	{
		setBlockBounds(6.0f / 16.0f, 0.0f, 0.0f, 10.0f / 16.0f, 1.0f, 1.0f);
	}
	else
	{
		setBlockBounds(0.0f, 0.0f, 6.0f / 16.0f, 1.0f, 1.0f, 10.0f / 16.0f);
	}
}

bool BlockFenceGate::isOpaqueCube()
{
	return false;
}

bool BlockFenceGate::renderAsNormalBlock()
{
	return false;
}

bool BlockFenceGate::getBlocksMovement(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z)
{
	return isFenceGateOpen(iblockaccess->getBlockMetadata(x, y, z));
}

int_t BlockFenceGate::getRenderType()
{
	return 21;
}

void BlockFenceGate::onBlockPlacedBy(World *world, int_t x, int_t y, int_t z, EntityLiving *entityliving)
{
	const int_t direction = (MathHelper::floor_double((double)(entityliving->rotationYaw * 4.0f / 360.0f) + 0.5) & 3) % 4;
	world->setBlockMetadataWithNotify(x, y, z, direction);
}

bool BlockFenceGate::blockActivated(World *world, int_t x, int_t y, int_t z, EntityPlayer *entityplayer)
{
	int_t metadata = world->getBlockMetadata(x, y, z);
	if (isFenceGateOpen(metadata))
	{
		world->setBlockMetadataWithNotify(x, y, z, metadata & -5);
	}
	else
	{
		const int_t playerDirection = (MathHelper::floor_double((double)(entityplayer->rotationYaw * 4.0f / 360.0f) + 0.5) & 3) % 4;
		const int_t gateDirection = getDirection(metadata);
		if (gateDirection == (playerDirection + 2) % 4)
		{
			metadata = playerDirection;
		}
		world->setBlockMetadataWithNotify(x, y, z, metadata | 4);
	}
	world->playAuxSFXAtEntity(entityplayer, 1003, x, y, z, 0);
	return true;
}

void BlockFenceGate::onNeighborBlockChange(World *world, int_t x, int_t y, int_t z, int_t neighborId)
{
	if (world->multiplayerWorld)
	{
		return;
	}
	const int_t metadata = world->getBlockMetadata(x, y, z);
	const bool powered = world->isBlockIndirectlyGettingPowered(x, y, z);
	const bool neighborCanPower = neighborId > 0 && neighborId < BLOCK_REGISTRY_SIZE && blocksList[neighborId] != nullptr && blocksList[neighborId]->canProvidePower();
	if (powered || neighborCanPower || neighborId == 0)
	{
		if (powered && !isFenceGateOpen(metadata))
		{
			world->setBlockMetadataWithNotify(x, y, z, metadata | 4);
			world->playAuxSFXAtEntity(nullptr, 1003, x, y, z, 0);
		}
		else if (!powered && isFenceGateOpen(metadata))
		{
			world->setBlockMetadataWithNotify(x, y, z, metadata & -5);
			world->playAuxSFXAtEntity(nullptr, 1003, x, y, z, 0);
		}
	}
}

bool BlockFenceGate::isFenceGateOpen(int_t metadata)
{
	return (metadata & 4) != 0;
}
