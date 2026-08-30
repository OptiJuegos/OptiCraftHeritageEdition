#include "BlockRedstoneWire.h"

#include "IBlockAccess.h"
#include "Item.h"
#include "Material.h"
#include "Direction.h"
#include "World.h"
#include "java/Random.h"

BlockRedstoneWire::BlockRedstoneWire(int_t i, int_t j) :
	Block(i, j, Material::circuits),
	wiresProvidePower(true)
{
	setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.0625f, 1.0f);
}

int_t BlockRedstoneWire::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
	return blockIndexInTexture;
}

AxisAlignedBB *BlockRedstoneWire::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	return nullptr;
}

bool BlockRedstoneWire::isOpaqueCube()
{
	return false;
}

bool BlockRedstoneWire::renderAsNormalBlock()
{
	return false;
}

int_t BlockRedstoneWire::getRenderType()
{
	return 5;
}

int_t BlockRedstoneWire::colorMultiplier(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	return 0x800000;
}

bool BlockRedstoneWire::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
	return world->isBlockNormalCube(i, j - 1, k) || world->getBlockId(i, j - 1, k) == Block::glowStone->blockID;
}

void BlockRedstoneWire::updateAndPropagateCurrentStrength(World *world, int_t i, int_t j, int_t k)
{
	updateCurrentStrength(world, i, j, k, i, j, k);
	std::vector<ChunkPosition> arraylist = pendingWireUpdates.valuesInIterationOrder();
	pendingWireUpdates.clear();
	for (const ChunkPosition &position : arraylist)
	{
		world->notifyBlocksOfNeighborChange(position.x, position.y, position.z, blockID);
	}
}

void BlockRedstoneWire::updateCurrentStrength(World *world, int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1)
{
	int_t k1 = world->getBlockMetadata(i, j, k);
	int_t l1 = 0;
	wiresProvidePower = false;
	bool flag = world->isBlockIndirectlyGettingPowered(i, j, k);
	wiresProvidePower = true;
	if (flag)
	{
		l1 = 15;
	}
	else
	{
		for (int_t i2 = 0; i2 < 4; i2++)
		{
			int_t k2 = i;
			int_t i3 = k;
			if (i2 == 0) k2--;
			if (i2 == 1) k2++;
			if (i2 == 2) i3--;
			if (i2 == 3) i3++;
			if (k2 != l || j != i1 || i3 != j1)
			{
				l1 = getMaxCurrentStrength(world, k2, j, i3, l1);
			}
			if (world->isBlockNormalCube(k2, j, i3) && !world->isBlockNormalCube(i, j + 1, k))
			{
				if (k2 != l || j + 1 != i1 || i3 != j1)
				{
					l1 = getMaxCurrentStrength(world, k2, j + 1, i3, l1);
				}
				continue;
			}
			if (!world->isBlockNormalCube(k2, j, i3) && (k2 != l || j - 1 != i1 || i3 != j1))
			{
				l1 = getMaxCurrentStrength(world, k2, j - 1, i3, l1);
			}
		}

		if (l1 > 0)
		{
			l1--;
		}
		else
		{
			l1 = 0;
		}
	}
	if (k1 != l1)
	{
		world->editingBlocks = true;
		world->setBlockMetadataWithNotify(i, j, k, l1);
		world->markBlocksDirty(i, j, k, i, j, k);
		world->editingBlocks = false;
		for (int_t j2 = 0; j2 < 4; j2++)
		{
			int_t l2 = i;
			int_t j3 = k;
			int_t k3 = j - 1;
			if (j2 == 0) l2--;
			if (j2 == 1) l2++;
			if (j2 == 2) j3--;
			if (j2 == 3) j3++;
			if (world->isBlockNormalCube(l2, j, j3))
			{
				k3 += 2;
			}
			int_t l3 = 0;
			l3 = getMaxCurrentStrength(world, l2, j, j3, -1);
			l1 = world->getBlockMetadata(i, j, k);
			if (l1 > 0)
			{
				l1--;
			}
			if (l3 >= 0 && l3 != l1)
			{
				updateCurrentStrength(world, l2, j, j3, i, j, k);
			}
			l3 = getMaxCurrentStrength(world, l2, k3, j3, -1);
			l1 = world->getBlockMetadata(i, j, k);
			if (l1 > 0)
			{
				l1--;
			}
			if (l3 >= 0 && l3 != l1)
			{
				updateCurrentStrength(world, l2, k3, j3, i, j, k);
			}
		}

		if (k1 < l1 || l1 == 0)
		{
			pendingWireUpdates.add(ChunkPosition(i, j, k));
			pendingWireUpdates.add(ChunkPosition(i - 1, j, k));
			pendingWireUpdates.add(ChunkPosition(i + 1, j, k));
			pendingWireUpdates.add(ChunkPosition(i, j - 1, k));
			pendingWireUpdates.add(ChunkPosition(i, j + 1, k));
			pendingWireUpdates.add(ChunkPosition(i, j, k - 1));
			pendingWireUpdates.add(ChunkPosition(i, j, k + 1));
		}
	}
}

void BlockRedstoneWire::notifyWireNeighborsOfNeighborChange(World *world, int_t i, int_t j, int_t k)
{
	if (world->getBlockId(i, j, k) != blockID)
	{
		return;
	}
	else
	{
		world->notifyBlocksOfNeighborChange(i, j, k, blockID);
		world->notifyBlocksOfNeighborChange(i - 1, j, k, blockID);
		world->notifyBlocksOfNeighborChange(i + 1, j, k, blockID);
		world->notifyBlocksOfNeighborChange(i, j, k - 1, blockID);
		world->notifyBlocksOfNeighborChange(i, j, k + 1, blockID);
		world->notifyBlocksOfNeighborChange(i, j - 1, k, blockID);
		world->notifyBlocksOfNeighborChange(i, j + 1, k, blockID);
		return;
	}
}

void BlockRedstoneWire::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
	Block::onBlockAdded(world, i, j, k);
	if (world->multiplayerWorld)
	{
		return;
	}
	updateAndPropagateCurrentStrength(world, i, j, k);
	world->notifyBlocksOfNeighborChange(i, j + 1, k, blockID);
	world->notifyBlocksOfNeighborChange(i, j - 1, k, blockID);
	notifyWireNeighborsOfNeighborChange(world, i - 1, j, k);
	notifyWireNeighborsOfNeighborChange(world, i + 1, j, k);
	notifyWireNeighborsOfNeighborChange(world, i, j, k - 1);
	notifyWireNeighborsOfNeighborChange(world, i, j, k + 1);
	if (world->isBlockNormalCube(i - 1, j, k)) notifyWireNeighborsOfNeighborChange(world, i - 1, j + 1, k);
	else notifyWireNeighborsOfNeighborChange(world, i - 1, j - 1, k);
	if (world->isBlockNormalCube(i + 1, j, k)) notifyWireNeighborsOfNeighborChange(world, i + 1, j + 1, k);
	else notifyWireNeighborsOfNeighborChange(world, i + 1, j - 1, k);
	if (world->isBlockNormalCube(i, j, k - 1)) notifyWireNeighborsOfNeighborChange(world, i, j + 1, k - 1);
	else notifyWireNeighborsOfNeighborChange(world, i, j - 1, k - 1);
	if (world->isBlockNormalCube(i, j, k + 1)) notifyWireNeighborsOfNeighborChange(world, i, j + 1, k + 1);
	else notifyWireNeighborsOfNeighborChange(world, i, j - 1, k + 1);
}

void BlockRedstoneWire::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
	Block::onBlockRemoval(world, i, j, k);
	if (world->multiplayerWorld)
	{
		return;
	}
	world->notifyBlocksOfNeighborChange(i, j + 1, k, blockID);
	world->notifyBlocksOfNeighborChange(i, j - 1, k, blockID);
	world->notifyBlocksOfNeighborChange(i + 1, j, k, blockID);
	world->notifyBlocksOfNeighborChange(i - 1, j, k, blockID);
	world->notifyBlocksOfNeighborChange(i, j, k + 1, blockID);
	world->notifyBlocksOfNeighborChange(i, j, k - 1, blockID);
	updateAndPropagateCurrentStrength(world, i, j, k);
	notifyWireNeighborsOfNeighborChange(world, i - 1, j, k);
	notifyWireNeighborsOfNeighborChange(world, i + 1, j, k);
	notifyWireNeighborsOfNeighborChange(world, i, j, k - 1);
	notifyWireNeighborsOfNeighborChange(world, i, j, k + 1);
	if (world->isBlockNormalCube(i - 1, j, k)) notifyWireNeighborsOfNeighborChange(world, i - 1, j + 1, k);
	else notifyWireNeighborsOfNeighborChange(world, i - 1, j - 1, k);
	if (world->isBlockNormalCube(i + 1, j, k)) notifyWireNeighborsOfNeighborChange(world, i + 1, j + 1, k);
	else notifyWireNeighborsOfNeighborChange(world, i + 1, j - 1, k);
	if (world->isBlockNormalCube(i, j, k - 1)) notifyWireNeighborsOfNeighborChange(world, i, j + 1, k - 1);
	else notifyWireNeighborsOfNeighborChange(world, i, j - 1, k - 1);
	if (world->isBlockNormalCube(i, j, k + 1)) notifyWireNeighborsOfNeighborChange(world, i, j + 1, k + 1);
	else notifyWireNeighborsOfNeighborChange(world, i, j - 1, k + 1);
}

int_t BlockRedstoneWire::getMaxCurrentStrength(World *world, int_t i, int_t j, int_t k, int_t l)
{
	if (world->getBlockId(i, j, k) != blockID)
	{
		return l;
	}
	int_t i1 = world->getBlockMetadata(i, j, k);
	if (i1 > l)
	{
		return i1;
	}
	else
	{
		return l;
	}
}

void BlockRedstoneWire::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	if (world->multiplayerWorld)
	{
		return;
	}
	int_t i1 = world->getBlockMetadata(i, j, k);
	bool flag = canPlaceBlockAt(world, i, j, k);
	if (!flag)
	{
		dropBlockAsItem(world, i, j, k, i1);
		world->setBlockWithNotify(i, j, k, 0);
	}
	else
	{
		updateAndPropagateCurrentStrength(world, i, j, k);
	}
	Block::onNeighborBlockChange(world, i, j, k, l);
}

int_t BlockRedstoneWire::idDropped(int_t i, Random &random)
{
	return Item::redstone->shiftedIndex;
}

bool BlockRedstoneWire::isIndirectlyPoweringTo(World *world, int_t i, int_t j, int_t k, int_t l)
{
	if (!wiresProvidePower)
	{
		return false;
	}
	else
	{
		return isPoweringTo(world, i, j, k, l);
	}
}

bool BlockRedstoneWire::isPoweringTo(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	if (!wiresProvidePower)
	{
		return false;
	}
	if (iblockaccess->getBlockMetadata(i, j, k) == 0)
	{
		return false;
	}
	if (l == 1)
	{
		return true;
	}
	bool flag = isPowerProviderOrWire(iblockaccess, i - 1, j, k, 1) || !iblockaccess->isBlockNormalCube(i - 1, j, k) && isPowerProviderOrWire(iblockaccess, i - 1, j - 1, k, -1);
	bool flag1 = isPowerProviderOrWire(iblockaccess, i + 1, j, k, 3) || !iblockaccess->isBlockNormalCube(i + 1, j, k) && isPowerProviderOrWire(iblockaccess, i + 1, j - 1, k, -1);
	bool flag2 = isPowerProviderOrWire(iblockaccess, i, j, k - 1, 2) || !iblockaccess->isBlockNormalCube(i, j, k - 1) && isPowerProviderOrWire(iblockaccess, i, j - 1, k - 1, -1);
	bool flag3 = isPowerProviderOrWire(iblockaccess, i, j, k + 1, 0) || !iblockaccess->isBlockNormalCube(i, j, k + 1) && isPowerProviderOrWire(iblockaccess, i, j - 1, k + 1, -1);
	if (!iblockaccess->isBlockNormalCube(i, j + 1, k))
	{
		if (iblockaccess->isBlockNormalCube(i - 1, j, k) && isPowerProviderOrWire(iblockaccess, i - 1, j + 1, k, -1)) flag = true;
		if (iblockaccess->isBlockNormalCube(i + 1, j, k) && isPowerProviderOrWire(iblockaccess, i + 1, j + 1, k, -1)) flag1 = true;
		if (iblockaccess->isBlockNormalCube(i, j, k - 1) && isPowerProviderOrWire(iblockaccess, i, j + 1, k - 1, -1)) flag2 = true;
		if (iblockaccess->isBlockNormalCube(i, j, k + 1) && isPowerProviderOrWire(iblockaccess, i, j + 1, k + 1, -1)) flag3 = true;
	}
	if (!flag2 && !flag1 && !flag && !flag3 && l >= 2 && l <= 5)
	{
		return true;
	}
	if (l == 2 && flag2 && !flag && !flag1)
	{
		return true;
	}
	if (l == 3 && flag3 && !flag && !flag1)
	{
		return true;
	}
	if (l == 4 && flag && !flag2 && !flag3)
	{
		return true;
	}
	return l == 5 && flag1 && !flag2 && !flag3;
}

bool BlockRedstoneWire::canProvidePower()
{
	return wiresProvidePower;
}

void BlockRedstoneWire::randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	int_t l = world->getBlockMetadata(i, j, k);
	if (l > 0)
	{
		double d = (double)i + 0.5 + ((double)random.nextFloat() - 0.5) * 0.20000000000000001;
		double d1 = (float)j + 0.0625f;
		double d2 = (double)k + 0.5 + ((double)random.nextFloat() - 0.5) * 0.20000000000000001;
		float f = (float)l / 15.0f;
		float f1 = f * 0.6f + 0.4f;
		if (l == 0)
		{
			f1 = 0.0f;
		}
		float f2 = f * f * 0.7f - 0.5f;
		float f3 = f * f * 0.6f - 0.7f;
		if (f2 < 0.0f) f2 = 0.0f;
		if (f3 < 0.0f) f3 = 0.0f;
		world->spawnParticle("reddust", d, d1, d2, f1, f2, f3);
	}
}

bool BlockRedstoneWire::isPowerProviderOrWire(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	int_t i1 = iblockaccess->getBlockId(i, j, k);
	if (i1 == Block::redstoneWire->blockID)
	{
		return true;
	}
	if (i1 == 0)
	{
		return false;
	}
	if (i1 != Block::redstoneRepeaterIdle->blockID && i1 != Block::redstoneRepeaterActive->blockID)
	{
		return Block::blocksList[i1]->canProvidePower() && l != -1;
	}

	const int_t metadata = iblockaccess->getBlockMetadata(i, j, k) & 3;
	return l == metadata || l == Direction::footInvisibleFaceRemap[metadata];
}
