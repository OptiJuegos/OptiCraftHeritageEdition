#include "BlockMushroom.h"
#include "java/Arithmetic.h"

#include "Block.h"
#include "BlockFlower.h"
#include "BlockMycelium.h"
#include "World.h"
#include "WorldGenBigMushroom.h"

BlockMushroom::BlockMushroom(int_t i, int_t j) :
	BlockFlower(i, j)
{
	float f = 0.2f;
	setBlockBounds(0.5f - f, 0.0f, 0.5f - f, 0.5f + f, f * 2.0f, 0.5f + f);
	setTickOnLoad(true);
}

void BlockMushroom::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	if (random.nextInt(25) != 0)
		return;

	const int_t radius = 4;
	int_t remaining = 5;
	for (int_t x = i - radius; x <= i + radius; ++x)
	{
		for (int_t z = k - radius; z <= k + radius; ++z)
		{
			for (int_t y = j - 1; y <= j + 1; ++y)
			{
				if (world->getBlockId(x, y, z) == blockID && --remaining <= 0)
					return;
			}
		}
	}

	int_t x = i + random.nextInt(3) - 1;
	int_t y = JavaArithmetic::intAdd(j, random.nextIntDifference(2));
	int_t z = k + random.nextInt(3) - 1;

	for (int_t attempt = 0; attempt < 4; ++attempt)
	{
		if (world->isAirBlock(x, y, z) && canBlockStay(world, x, y, z))
		{
			i = x;
			j = y;
			k = z;
		}

		x = i + random.nextInt(3) - 1;
		y = JavaArithmetic::intAdd(j, random.nextIntDifference(2));
		z = k + random.nextInt(3) - 1;
	}

	if (world->isAirBlock(x, y, z) && canBlockStay(world, x, y, z))
		world->setBlockWithNotify(x, y, z, blockID);
}

bool BlockMushroom::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
	return BlockFlower::canPlaceBlockAt(world, i, j, k) && canBlockStay(world, i, j, k);
}

bool BlockMushroom::canThisPlantGrowOnThisBlockID(int_t i)
{
	return i >= 0 && i < Block::BLOCK_REGISTRY_SIZE && Block::opaqueCubeLookup[i];
}

bool BlockMushroom::canBlockStay(World *world, int_t i, int_t j, int_t k)
{
	if (j < 0 || j >= 256)
		return false;

	int_t blockBelow = world->getBlockId(i, j - 1, k);
	return blockBelow == Block::mycelium->blockID ||
	       (world->getFullBlockLightValue(i, j, k) < 13 && canThisPlantGrowOnThisBlockID(blockBelow));
}

bool BlockMushroom::fertilizeMushroom(World *world, int_t i, int_t j, int_t k, Random &random)
{
	int_t metadata = world->getBlockMetadata(i, j, k);
	world->setBlock(i, j, k, 0);

	WorldGenBigMushroom *generator = nullptr;
	if (blockID == Block::mushroomBrown->blockID)
		generator = new WorldGenBigMushroom(0);
	else if (blockID == Block::mushroomRed->blockID)
		generator = new WorldGenBigMushroom(1);

	bool generated = generator != nullptr && generator->generate(world, random, i, j, k);
	delete generator;
	if (generated)
		return true;

	world->setBlockAndMetadata(i, j, k, blockID, metadata);
	return false;
}
