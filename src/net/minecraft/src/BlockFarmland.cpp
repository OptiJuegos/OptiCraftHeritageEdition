#include "BlockFarmland.h"
#include "Material.h"
#include "AxisAlignedBB.h"
#include "World.h"
#include "Block.h"
#include "Entity.h"

BlockFarmland::BlockFarmland(int_t i) : Block(i, Material::ground)
{
	blockIndexInTexture = 87;
	setTickOnLoad(true);
	setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.9375f, 1.0f);
	setLightOpacity(255);
}

AxisAlignedBB *BlockFarmland::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	return AxisAlignedBB::getBoundingBoxFromPool(i + 0, j + 0, k + 0, i + 1, j + 1, k + 1);
}

bool  BlockFarmland::isOpaqueCube()        { return false; }
bool  BlockFarmland::renderAsNormalBlock() { return false; }

int_t BlockFarmland::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
	if (i == 1 && j > 0) return blockIndexInTexture - 1;
	if (i == 1)          return blockIndexInTexture;
	return 2;
}

void BlockFarmland::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	(void)random;
	if (!isWaterNearby(world, i, j, k) && !world->canLightningStrikeAt(i, j + 1, k))
	{
		int_t l = world->getBlockMetadata(i, j, k);
		if (l > 0)
		{
			world->setBlockMetadataWithNotify(i, j, k, l - 1);
		}
		else if (!isCropsNearby(world, i, j, k))
		{
			world->setBlockWithNotify(i, j, k, Block::dirt->blockID);
		}
	}
	else
	{
		world->setBlockMetadataWithNotify(i, j, k, 7);
	}
}

void BlockFarmland::onFallenUpon(World *world, int_t i, int_t j, int_t k, Entity *entity, float fallDistance)
{
	(void)entity;
	if (world->rand.nextFloat() < fallDistance - 0.5f)
		world->setBlockWithNotify(i, j, k, Block::dirt->blockID);
}

void BlockFarmland::onEntityWalking(World *world, int_t i, int_t j, int_t k, Entity *entity)
{
	(void)world;
	(void)i;
	(void)j;
	(void)k;
	(void)entity;
}

bool BlockFarmland::isCropsNearby(World *world, int_t i, int_t j, int_t k)
{
	int_t l = 0;
	for (int_t i1 = i - l; i1 <= i + l; i1++)
	{
		for (int_t j1 = k - l; j1 <= k + l; j1++)
		{
			int_t blockId = world->getBlockId(i1, j + 1, j1);
			if (blockId == Block::crops->blockID || blockId == Block::melonStem->blockID || blockId == Block::pumpkinStem->blockID)
				return true;
		}
	}
	return false;
}

bool BlockFarmland::isWaterNearby(World *world, int_t i, int_t j, int_t k)
{
	for (int_t l = i - 4; l <= i + 4; l++)
	{
		for (int_t i1 = j; i1 <= j + 1; i1++)
		{
			for (int_t j1 = k - 4; j1 <= k + 4; j1++)
			{
				if (world->getBlockMaterial(l, i1, j1) == Material::water) return true;
			}
		}
	}
	return false;
}

void BlockFarmland::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	Block::onNeighborBlockChange(world, i, j, k, l);
	Material *material = world->getBlockMaterial(i, j + 1, k);
	if (material->isSolid())
	{
		world->setBlockWithNotify(i, j, k, Block::dirt->blockID);
	}
}

int_t BlockFarmland::idDropped(int_t i, Random &random)
{
	return Block::dirt->idDropped(0, random);
}

int_t BlockFarmland::idDropped(int_t i, Random &random, int_t fortune)
{
	(void)i;
	return Block::dirt->idDropped(0, random, fortune);
}
