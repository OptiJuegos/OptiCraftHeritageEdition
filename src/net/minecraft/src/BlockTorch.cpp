#include "BlockTorch.h"
#include "Material.h"
#include "World.h"
#include "Block.h"
#include "BlockStairs.h"
#include "AxisAlignedBB.h"
#include "Vec3D.h"
#include "MovingObjectPosition.h"

BlockTorch::BlockTorch(int_t i, int_t j) : Block(i, j, Material::circuits)
{
	setTickOnLoad(true);
}

AxisAlignedBB *BlockTorch::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	return nullptr;
}

bool  BlockTorch::isOpaqueCube()        { return false; }
bool  BlockTorch::renderAsNormalBlock() { return false; }
int_t BlockTorch::getRenderType()       { return 2; }

bool BlockTorch::getTorchTexture(World *world, int_t i, int_t j, int_t k)
{
	if (world->isBlockNormalCubeDefault(i, j, k, true))
		return true;

	const int_t blockId = world->getBlockId(i, j, k);
	if (blockId == Block::fence->blockID || blockId == Block::netherFence->blockID || blockId == Block::glass->blockID)
		return true;

	Block *block = blockId >= 0 && blockId < BLOCK_REGISTRY_SIZE ? Block::blocksList[blockId] : nullptr;
	if (dynamic_cast<BlockStairs *>(block) != nullptr)
		return (world->getBlockMetadata(i, j, k) & 4) != 0;

	return false;
}

bool BlockTorch::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
	if (world->isBlockNormalCubeDefault(i - 1, j, k, true)) return true;
	if (world->isBlockNormalCubeDefault(i + 1, j, k, true)) return true;
	if (world->isBlockNormalCubeDefault(i, j, k - 1, true)) return true;
	if (world->isBlockNormalCubeDefault(i, j, k + 1, true)) return true;
	return getTorchTexture(world, i, j - 1, k);
}

void BlockTorch::onBlockPlaced(World *world, int_t i, int_t j, int_t k, int_t l)
{
	int_t i1 = world->getBlockMetadata(i, j, k);
	if (l == 1 && getTorchTexture(world, i, j - 1, k)) i1 = 5;
	if (l == 2 && world->isBlockNormalCubeDefault(i, j, k + 1, true)) i1 = 4;
	if (l == 3 && world->isBlockNormalCubeDefault(i, j, k - 1, true)) i1 = 3;
	if (l == 4 && world->isBlockNormalCubeDefault(i + 1, j, k, true)) i1 = 2;
	if (l == 5 && world->isBlockNormalCubeDefault(i - 1, j, k, true)) i1 = 1;
	world->setBlockMetadataWithNotify(i, j, k, i1);
}

void BlockTorch::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	Block::updateTick(world, i, j, k, random);
	if (world->getBlockMetadata(i, j, k) == 0)
	{
		onBlockAdded(world, i, j, k);
	}
}

void BlockTorch::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
	if (world->isBlockNormalCubeDefault(i - 1, j, k, true))      world->setBlockMetadataWithNotify(i, j, k, 1);
	else if (world->isBlockNormalCubeDefault(i + 1, j, k, true)) world->setBlockMetadataWithNotify(i, j, k, 2);
	else if (world->isBlockNormalCubeDefault(i, j, k - 1, true)) world->setBlockMetadataWithNotify(i, j, k, 3);
	else if (world->isBlockNormalCubeDefault(i, j, k + 1, true)) world->setBlockMetadataWithNotify(i, j, k, 4);
	else if (getTorchTexture(world, i, j - 1, k))      world->setBlockMetadataWithNotify(i, j, k, 5);
	dropTorchIfCantStay(world, i, j, k);
}

void BlockTorch::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	if (dropTorchIfCantStay(world, i, j, k))
	{
		int_t i1 = world->getBlockMetadata(i, j, k);
		bool flag = false;
		if (!world->isBlockNormalCubeDefault(i - 1, j, k, true) && i1 == 1) flag = true;
		if (!world->isBlockNormalCubeDefault(i + 1, j, k, true) && i1 == 2) flag = true;
		if (!world->isBlockNormalCubeDefault(i, j, k - 1, true) && i1 == 3) flag = true;
		if (!world->isBlockNormalCubeDefault(i, j, k + 1, true) && i1 == 4) flag = true;
		if (!getTorchTexture(world, i, j - 1, k) && i1 == 5)      flag = true;
		if (flag)
		{
			dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
			world->setBlockWithNotify(i, j, k, 0);
		}
	}
}

bool BlockTorch::dropTorchIfCantStay(World *world, int_t i, int_t j, int_t k)
{
	if (!canPlaceBlockAt(world, i, j, k))
	{
		if (world->getBlockId(i, j, k) == blockID)
		{
			dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
			world->setBlockWithNotify(i, j, k, 0);
		}
		return false;
	}
	return true;
}

MovingObjectPosition *BlockTorch::collisionRayTrace(World *world, int_t i, int_t j, int_t k, Vec3D *vec3d, Vec3D *vec3d1)
{
	int_t l = world->getBlockMetadata(i, j, k) & 7;
	float f = 0.15f;
	if (l == 1)      setBlockBounds(0.0f, 0.2f, 0.5f - f, f * 2.0f, 0.8f, 0.5f + f);
	else if (l == 2) setBlockBounds(1.0f - f * 2.0f, 0.2f, 0.5f - f, 1.0f, 0.8f, 0.5f + f);
	else if (l == 3) setBlockBounds(0.5f - f, 0.2f, 0.0f, 0.5f + f, 0.8f, f * 2.0f);
	else if (l == 4) setBlockBounds(0.5f - f, 0.2f, 1.0f - f * 2.0f, 0.5f + f, 0.8f, 1.0f);
	else
	{
		float f1 = 0.1f;
		setBlockBounds(0.5f - f1, 0.0f, 0.5f - f1, 0.5f + f1, 0.6f, 0.5f + f1);
	}
	return Block::collisionRayTrace(world, i, j, k, vec3d, vec3d1);
}

void BlockTorch::randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	int_t l = world->getBlockMetadata(i, j, k);
	double d  = (float)i + 0.5f;
	double d1 = (float)j + 0.7f;
	double d2 = (float)k + 0.5f;
	double d3 = 0.2199999988079071;
	double d4 = 0.27000001072883606;
	if (l == 1)
	{
		world->spawnParticle("smoke", d - d4, d1 + d3, d2, 0.0, 0.0, 0.0);
		world->spawnParticle("flame", d - d4, d1 + d3, d2, 0.0, 0.0, 0.0);
	}
	else if (l == 2)
	{
		world->spawnParticle("smoke", d + d4, d1 + d3, d2, 0.0, 0.0, 0.0);
		world->spawnParticle("flame", d + d4, d1 + d3, d2, 0.0, 0.0, 0.0);
	}
	else if (l == 3)
	{
		world->spawnParticle("smoke", d, d1 + d3, d2 - d4, 0.0, 0.0, 0.0);
		world->spawnParticle("flame", d, d1 + d3, d2 - d4, 0.0, 0.0, 0.0);
	}
	else if (l == 4)
	{
		world->spawnParticle("smoke", d, d1 + d3, d2 + d4, 0.0, 0.0, 0.0);
		world->spawnParticle("flame", d, d1 + d3, d2 + d4, 0.0, 0.0, 0.0);
	}
	else
	{
		world->spawnParticle("smoke", d, d1, d2, 0.0, 0.0, 0.0);
		world->spawnParticle("flame", d, d1, d2, 0.0, 0.0, 0.0);
	}
}
