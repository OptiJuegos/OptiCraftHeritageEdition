#include "BlockCactus.h"
#include "DamageSource.h"
#include "Material.h"
#include "World.h"
#include "AxisAlignedBB.h"
#include "Block.h"
#include "Entity.h"

BlockCactus::BlockCactus(int_t i, int_t j) : Block(i, j, Material::cactus)
{
	setTickOnLoad(true);
}

void BlockCactus::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
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

AxisAlignedBB *BlockCactus::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	float f = 0.0625f;
	return AxisAlignedBB::getBoundingBoxFromPool((float)i + f, j, (float)k + f,
	                                             (float)(i + 1) - f, (float)(j + 1) - f, (float)(k + 1) - f);
}

AxisAlignedBB *BlockCactus::getSelectedBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	float f = 0.0625f;
	return AxisAlignedBB::getBoundingBoxFromPool((float)i + f, j, (float)k + f,
	                                             (float)(i + 1) - f, j + 1, (float)(k + 1) - f);
}

int_t BlockCactus::getBlockTextureFromSide(int_t i)
{
	if (i == 1) return blockIndexInTexture - 1;
	if (i == 0) return blockIndexInTexture + 1;
	return blockIndexInTexture;
}

bool  BlockCactus::renderAsNormalBlock() { return false; }
bool  BlockCactus::isOpaqueCube()        { return false; }
int_t BlockCactus::getRenderType()       { return 13; }

bool BlockCactus::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
	if (!Block::canPlaceBlockAt(world, i, j, k)) return false;
	return canBlockStay(world, i, j, k);
}

void BlockCactus::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	if (!canBlockStay(world, i, j, k))
	{
		dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
		world->setBlockWithNotify(i, j, k, 0);
	}
}

bool BlockCactus::canBlockStay(World *world, int_t i, int_t j, int_t k)
{
	if (world->getBlockMaterial(i - 1, j, k)->isSolid()) return false;
	if (world->getBlockMaterial(i + 1, j, k)->isSolid()) return false;
	if (world->getBlockMaterial(i, j, k - 1)->isSolid()) return false;
	if (world->getBlockMaterial(i, j, k + 1)->isSolid()) return false;
	int_t l = world->getBlockId(i, j - 1, k);
	return l == Block::cactus->blockID || l == Block::sand->blockID;
}

void BlockCactus::onEntityCollidedWithBlock(World *world, int_t i, int_t j, int_t k, Entity *entity)
{
	entity->attackEntityFrom(DamageSource::cactus, 1);
}
