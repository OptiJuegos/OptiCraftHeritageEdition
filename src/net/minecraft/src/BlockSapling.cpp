#include "BlockSapling.h"
#include "World.h"
#include "WorldGenTaiga2.h"
#include "WorldGenForest.h"
#include "WorldGenTrees.h"
#include "WorldGenBigTree.h"
#include "WorldGenHugeTrees.h"

BlockSapling::BlockSapling(int_t i, int_t j) : BlockFlower(i, j)
{
	float f = 0.4f;
	setBlockBounds(0.5f - f, 0.0f, 0.5f - f, 0.5f + f, f * 2.0f, 0.5f + f);
}

void BlockSapling::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	if (world->multiplayerWorld) return;
	BlockFlower::updateTick(world, i, j, k, random);
	if (world->getBlockLightValue(i, j + 1, k) >= 9 && random.nextInt(7) == 0)
	{
		int_t l = world->getBlockMetadata(i, j, k);
		if ((l & 8) == 0)
		{
			world->setBlockMetadataWithNotify(i, j, k, l | 8);
		}
		else
		{
			growTree(world, i, j, k, random);
		}
	}
}

int_t BlockSapling::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
	j &= 3;
	if (j == 1) return 63;
	if (j == 2) return 79;
	if (j == 3) return 30;
	return BlockFlower::getBlockTextureFromSideAndMetadata(i, j);
}

void BlockSapling::growTree(World *world, int_t i, int_t j, int_t k, Random &random)
{
	const int_t metadata = world->getBlockMetadata(i, j, k) & 3;
	WorldGenerator *generator = nullptr;
	int_t offsetX = 0;
	int_t offsetZ = 0;
	bool hugeJungle = false;

	if (metadata == 1)
	{
		generator = new WorldGenTaiga2(true);
	}
	else if (metadata == 2)
	{
		generator = new WorldGenForest(true);
	}
	else if (metadata == 3)
	{
		for (offsetX = 0; offsetX >= -1 && generator == nullptr; --offsetX)
		{
			for (offsetZ = 0; offsetZ >= -1; --offsetZ)
			{
				if (isSameSapling(world, i + offsetX, j, k + offsetZ, 3) &&
				    isSameSapling(world, i + offsetX + 1, j, k + offsetZ, 3) &&
				    isSameSapling(world, i + offsetX, j, k + offsetZ + 1, 3) &&
				    isSameSapling(world, i + offsetX + 1, j, k + offsetZ + 1, 3))
				{
					generator = new WorldGenHugeTrees(true, 10 + random.nextInt(20), 3, 3);
					hugeJungle = true;
					break;
				}
			}
		}

		if (generator == nullptr)
		{
			offsetX = 0;
			offsetZ = 0;
			generator = new WorldGenTrees(true, 4 + random.nextInt(7), 3, 3, false);
		}
	}
	else
	{
		generator = new WorldGenTrees(true);
		if (random.nextInt(10) == 0)
		{
			delete generator;
			generator = new WorldGenBigTree(true);
		}
	}

	if (hugeJungle)
	{
		world->setBlock(i + offsetX, j, k + offsetZ, 0);
		world->setBlock(i + offsetX + 1, j, k + offsetZ, 0);
		world->setBlock(i + offsetX, j, k + offsetZ + 1, 0);
		world->setBlock(i + offsetX + 1, j, k + offsetZ + 1, 0);
	}
	else
	{
		world->setBlock(i, j, k, 0);
	}

	if (!generator->generate(world, random, i + offsetX, j, k + offsetZ))
	{
		if (hugeJungle)
		{
			world->setBlockAndMetadata(i + offsetX, j, k + offsetZ, blockID, metadata);
			world->setBlockAndMetadata(i + offsetX + 1, j, k + offsetZ, blockID, metadata);
			world->setBlockAndMetadata(i + offsetX, j, k + offsetZ + 1, blockID, metadata);
			world->setBlockAndMetadata(i + offsetX + 1, j, k + offsetZ + 1, blockID, metadata);
		}
		else
		{
			world->setBlockAndMetadata(i, j, k, blockID, metadata);
		}
	}

	delete generator;
}

bool BlockSapling::isSameSapling(World *world, int_t i, int_t j, int_t k, int_t metadata)
{
	return world->getBlockId(i, j, k) == blockID && (world->getBlockMetadata(i, j, k) & 3) == metadata;
}

int_t BlockSapling::damageDropped(int_t i) { return i & 3; }
