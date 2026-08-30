#include "BlockStem.h"

#include "EntityItem.h"
#include "IBlockAccess.h"
#include "Item.h"
#include "ItemStack.h"
#include "World.h"
#include "BlockGrass.h"

BlockStem::BlockStem(int_t id, Block *fruit) :
	BlockFlower(id, 111),
	fruitType(fruit)
{
	setTickOnLoad(true);
	const float radius = 2.0f / 16.0f;
	setBlockBounds(0.5f - radius, 0.0f, 0.5f - radius, 0.5f + radius, 0.25f, 0.5f + radius);
}

bool BlockStem::canThisPlantGrowOnThisBlockID(int_t id)
{
	return id == Block::tilledField->blockID;
}

void BlockStem::updateTick(World *world, int_t x, int_t y, int_t z, Random &random)
{
	BlockFlower::updateTick(world, x, y, z, random);
	if (world->getBlockLightValue(x, y + 1, z) < 9)
	{
		return;
	}

	const float growthModifier = getGrowthModifier(world, x, y, z);
	if (random.nextInt((int_t)(25.0f / growthModifier) + 1) != 0)
	{
		return;
	}

	int_t metadata = world->getBlockMetadata(x, y, z);
	if (metadata < 7)
	{
		world->setBlockMetadataWithNotify(x, y, z, metadata + 1);
		return;
	}

	if (world->getBlockId(x - 1, y, z) == fruitType->blockID ||
		world->getBlockId(x + 1, y, z) == fruitType->blockID ||
		world->getBlockId(x, y, z - 1) == fruitType->blockID ||
		world->getBlockId(x, y, z + 1) == fruitType->blockID)
	{
		return;
	}

	int_t fruitX = x;
	int_t fruitZ = z;
	switch (random.nextInt(4))
	{
	case 0: --fruitX; break;
	case 1: ++fruitX; break;
	case 2: --fruitZ; break;
	case 3: ++fruitZ; break;
	}
	const int_t ground = world->getBlockId(fruitX, y - 1, fruitZ);
	if (world->getBlockId(fruitX, y, fruitZ) == 0 &&
		(ground == Block::tilledField->blockID || ground == Block::dirt->blockID || ground == Block::grass->blockID))
	{
		world->setBlockWithNotify(fruitX, y, fruitZ, fruitType->blockID);
	}
}

void BlockStem::fertilizeStem(World *world, int_t x, int_t y, int_t z)
{
	world->setBlockMetadataWithNotify(x, y, z, 7);
}

float BlockStem::getGrowthModifier(World *world, int_t x, int_t y, int_t z)
{
	float result = 1.0f;
	const int_t north = world->getBlockId(x, y, z - 1);
	const int_t south = world->getBlockId(x, y, z + 1);
	const int_t west = world->getBlockId(x - 1, y, z);
	const int_t east = world->getBlockId(x + 1, y, z);
	const int_t northWest = world->getBlockId(x - 1, y, z - 1);
	const int_t northEast = world->getBlockId(x + 1, y, z - 1);
	const int_t southEast = world->getBlockId(x + 1, y, z + 1);
	const int_t southWest = world->getBlockId(x - 1, y, z + 1);
	const bool horizontal = west == blockID || east == blockID;
	const bool vertical = north == blockID || south == blockID;
	const bool diagonal = northWest == blockID || northEast == blockID || southEast == blockID || southWest == blockID;

	for (int_t soilX = x - 1; soilX <= x + 1; ++soilX)
	{
		for (int_t soilZ = z - 1; soilZ <= z + 1; ++soilZ)
		{
			float contribution = 0.0f;
			if (world->getBlockId(soilX, y - 1, soilZ) == Block::tilledField->blockID)
			{
				contribution = world->getBlockMetadata(soilX, y - 1, soilZ) > 0 ? 3.0f : 1.0f;
			}
			if (soilX != x || soilZ != z)
			{
				contribution /= 4.0f;
			}
			result += contribution;
		}
	}

	if (diagonal || (horizontal && vertical))
	{
		result /= 2.0f;
	}
	return result;
}

int_t BlockStem::getRenderColor(int_t metadata)
{
	const int_t red = metadata * 32;
	const int_t green = 255 - metadata * 8;
	const int_t blue = metadata * 4;
	return red << 16 | green << 8 | blue;
}

int_t BlockStem::colorMultiplier(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z)
{
	return getRenderColor(iblockaccess->getBlockMetadata(x, y, z));
}

int_t BlockStem::getBlockTextureFromSideAndMetadata(int_t side, int_t metadata)
{
	return blockIndexInTexture;
}

void BlockStem::setBlockBoundsForItemRender()
{
	const float radius = 2.0f / 16.0f;
	setBlockBounds(0.5f - radius, 0.0f, 0.5f - radius, 0.5f + radius, 0.25f, 0.5f + radius);
}

void BlockStem::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z)
{
	const float height = (float)(iblockaccess->getBlockMetadata(x, y, z) * 2 + 2) / 16.0f;
	const float radius = 2.0f / 16.0f;
	setBlockBounds(0.5f - radius, 0.0f, 0.5f - radius, 0.5f + radius, height, 0.5f + radius);
}

int_t BlockStem::getRenderType()
{
	return 19;
}

int_t BlockStem::getStateForNeighbor(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z)
{
	const int_t metadata = iblockaccess->getBlockMetadata(x, y, z);
	if (metadata < 7)
	{
		return -1;
	}
	if (iblockaccess->getBlockId(x - 1, y, z) == fruitType->blockID) return 0;
	if (iblockaccess->getBlockId(x + 1, y, z) == fruitType->blockID) return 1;
	if (iblockaccess->getBlockId(x, y, z - 1) == fruitType->blockID) return 2;
	if (iblockaccess->getBlockId(x, y, z + 1) == fruitType->blockID) return 3;
	return -1;
}

void BlockStem::dropBlockAsItemWithChance(World *world, int_t x, int_t y, int_t z, int_t metadata, float chance, int_t fortune)
{
	BlockFlower::dropBlockAsItemWithChance(world, x, y, z, metadata, chance, fortune);
	if (world->multiplayerWorld)
	{
		return;
	}
	Item *seed = nullptr;
	if (fruitType == Block::pumpkin) seed = Item::pumpkinSeeds;
	if (fruitType == Block::melon) seed = Item::melonSeeds;
	if (seed == nullptr)
	{
		return;
	}

	for (int_t attempt = 0; attempt < 3; ++attempt)
	{
		if (world->rand.nextInt(15) <= metadata)
		{
			const float spread = 0.7f;
			const float offsetX = world->rand.nextFloat() * spread + (1.0f - spread) * 0.5f;
			const float offsetY = world->rand.nextFloat() * spread + (1.0f - spread) * 0.5f;
			const float offsetZ = world->rand.nextFloat() * spread + (1.0f - spread) * 0.5f;
			EntityItem *entity = new EntityItem(world, (double)x + offsetX, (double)y + offsetY, (double)z + offsetZ, new ItemStack(seed));
			entity->delayBeforeCanPickup = 10;
			if (!world->spawnEntityInWorld(entity))
				delete entity;
		}
	}
}

int_t BlockStem::idDropped(int_t metadata, Random &random, int_t fortune)
{
	return -1;
}

int_t BlockStem::quantityDropped(Random &random)
{
	return 1;
}
