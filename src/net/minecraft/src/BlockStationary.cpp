#include "BlockStationary.h"
#include "Material.h"
#include "World.h"
#include "Block.h"
#include "BlockFire.h"

BlockStationary::BlockStationary(int_t i, Material *material) : BlockFluid(i, material)
{
	setTickOnLoad(false);
	if (material == Material::lava)
	{
		setTickOnLoad(true);
	}
}

void BlockStationary::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	BlockFluid::onNeighborBlockChange(world, i, j, k, l);
	if (world->getBlockId(i, j, k) == blockID)
	{
		getLiquidMaterial(world, i, j, k);
	}
}

void BlockStationary::getLiquidMaterial(World *world, int_t i, int_t j, int_t k)
{
	int_t l = world->getBlockMetadata(i, j, k);
	world->editingBlocks = true;
	world->setBlockAndMetadata(i, j, k, blockID - 1, l);
	world->markBlocksDirty(i, j, k, i, j, k);
	world->scheduleBlockUpdate(i, j, k, blockID - 1, tickRate());
	world->editingBlocks = false;
}

void BlockStationary::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	if (blockMaterial != Material::lava)
		return;

	int_t attempts = random.nextInt(3);
	int_t attempt = 0;
	while (true)
	{
		if (attempt >= attempts)
		{
			if (attempts == 0)
			{
				int_t baseX = i;
				int_t baseZ = k;
				for (int_t lateralAttempt = 0; lateralAttempt < 3; ++lateralAttempt)
				{
					i = baseX + random.nextInt(3) - 1;
					k = baseZ + random.nextInt(3) - 1;
					if (world->isAirBlock(i, j + 1, k) && getIsStationary(world, i, j, k))
						world->setBlockWithNotify(i, j + 1, k, Block::fire->blockID);
				}
			}
			break;
		}

		i += random.nextInt(3) - 1;
		++j;
		k += random.nextInt(3) - 1;
		int_t blockId = world->getBlockId(i, j, k);
		if (blockId == 0)
		{
			if (getIsStationary(world, i - 1, j, k) || getIsStationary(world, i + 1, j, k) ||
			    getIsStationary(world, i, j, k - 1) || getIsStationary(world, i, j, k + 1) ||
			    getIsStationary(world, i, j - 1, k) || getIsStationary(world, i, j + 1, k))
			{
				world->setBlockWithNotify(i, j, k, Block::fire->blockID);
				return;
			}
		}
		else if (Block::blocksList[blockId]->blockMaterial->blocksMovement())
		{
			return;
		}

		++attempt;
	}
}

bool BlockStationary::getIsStationary(World *world, int_t i, int_t j, int_t k)
{
	return world->getBlockMaterial(i, j, k)->getBurning();
}

bool BlockStationary::getBlocksMovement(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	(void)iblockaccess;
	(void)i;
	(void)j;
	(void)k;
	return blockMaterial != Material::lava;
}
