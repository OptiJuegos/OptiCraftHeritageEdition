#include "BlockNetherStalk.h"

#include "BiomeGenBase.h"
#include "BiomeGenHell.h"
#include "Item.h"
#include "ItemStack.h"
#include "World.h"

BlockNetherStalk::BlockNetherStalk(int_t id) :
	BlockFlower(id, 226)
{
	setTickOnLoad(true);
	const float radius = 0.5f;
	setBlockBounds(0.5f - radius, 0.0f, 0.5f - radius, 0.5f + radius, 0.25f, 0.5f + radius);
}

bool BlockNetherStalk::canThisPlantGrowOnThisBlockID(int_t id)
{
	return id == Block::slowSand->blockID;
}

bool BlockNetherStalk::canBlockStay(World *world, int_t x, int_t y, int_t z)
{
	return canThisPlantGrowOnThisBlockID(world->getBlockId(x, y - 1, z));
}

void BlockNetherStalk::updateTick(World *world, int_t x, int_t y, int_t z, Random &random)
{
	int_t metadata = world->getBlockMetadata(x, y, z);
	if (metadata < 3)
	{
		BiomeGenBase *biome = world->getBiomeGenForCoords(x, z);
		if (dynamic_cast<BiomeGenHell *>(biome) != nullptr && random.nextInt(10) == 0)
		{
			world->setBlockMetadataWithNotify(x, y, z, metadata + 1);
		}
	}
	BlockFlower::updateTick(world, x, y, z, random);
}

int_t BlockNetherStalk::getBlockTextureFromSideAndMetadata(int_t side, int_t metadata)
{
	return metadata >= 3 ? blockIndexInTexture + 2 : (metadata > 0 ? blockIndexInTexture + 1 : blockIndexInTexture);
}

int_t BlockNetherStalk::getRenderType()
{
	return 6;
}

void BlockNetherStalk::dropBlockAsItemWithChance(World *world, int_t x, int_t y, int_t z, int_t metadata, float chance, int_t fortune)
{
	if (world->multiplayerWorld || Item::netherStalkSeeds == nullptr)
	{
		return;
	}
	int_t amount = 1;
	if (metadata >= 3)
	{
		amount = 2 + world->rand.nextInt(3);
		if (fortune > 0)
		{
			amount += world->rand.nextInt(fortune + 1);
		}
	}
	for (int_t i = 0; i < amount; ++i)
	{
		dropBlockAsItem_do(world, x, y, z, new ItemStack(Item::netherStalkSeeds));
	}
}

int_t BlockNetherStalk::idDropped(int_t metadata, Random &random, int_t fortune)
{
	return 0;
}

int_t BlockNetherStalk::quantityDropped(Random &random)
{
	return 0;
}
