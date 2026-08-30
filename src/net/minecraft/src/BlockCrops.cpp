#include "BlockCrops.h"
#include "Block.h"
#include "World.h"
#include "EntityItem.h"
#include "ItemStack.h"
#include "Item.h"

BlockCrops::BlockCrops(int_t i, int_t j) : BlockFlower(i, j)
{
	blockIndexInTexture = j;
	setTickOnLoad(true);
	float f = 0.5f;
	setBlockBounds(0.5f - f, 0.0f, 0.5f - f, 0.5f + f, 0.25f, 0.5f + f);
}

bool BlockCrops::canThisPlantGrowOnThisBlockID(int_t i)
{
	return i == Block::tilledField->blockID;
}

void BlockCrops::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	BlockFlower::updateTick(world, i, j, k, random);
	if (world->getBlockLightValue(i, j + 1, k) >= 9)
	{
		int_t l = world->getBlockMetadata(i, j, k);
		if (l < 7)
		{
			float f = getGrowthRate(world, i, j, k);
			if (random.nextInt((int_t)(25.0f / f) + 1) == 0)
			{
				l++;
				world->setBlockMetadataWithNotify(i, j, k, l);
			}
		}
	}
}

void BlockCrops::fertilize(World *world, int_t i, int_t j, int_t k)
{
	world->setBlockMetadataWithNotify(i, j, k, 7);
}

float BlockCrops::getGrowthRate(World *world, int_t i, int_t j, int_t k)
{
	float f = 1.0f;
	int_t l  = world->getBlockId(i, j, k - 1);
	int_t i1 = world->getBlockId(i, j, k + 1);
	int_t j1 = world->getBlockId(i - 1, j, k);
	int_t k1 = world->getBlockId(i + 1, j, k);
	int_t l1 = world->getBlockId(i - 1, j, k - 1);
	int_t i2 = world->getBlockId(i + 1, j, k - 1);
	int_t j2 = world->getBlockId(i + 1, j, k + 1);
	int_t k2 = world->getBlockId(i - 1, j, k + 1);
	bool flag  = j1 == blockID || k1 == blockID;
	bool flag1 = l == blockID  || i1 == blockID;
	bool flag2 = l1 == blockID || i2 == blockID || j2 == blockID || k2 == blockID;
	for (int_t l2 = i - 1; l2 <= i + 1; l2++)
	{
		for (int_t i3 = k - 1; i3 <= k + 1; i3++)
		{
			int_t j3 = world->getBlockId(l2, j - 1, i3);
			float f1 = 0.0f;
			if (j3 == Block::tilledField->blockID)
			{
				f1 = 1.0f;
				if (world->getBlockMetadata(l2, j - 1, i3) > 0) f1 = 3.0f;
			}
			if (l2 != i || i3 != k) f1 /= 4.0f;
			f += f1;
		}
	}
	if (flag2 || flag && flag1) f /= 2.0f;
	return f;
}

int_t BlockCrops::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
	if (j < 0) j = 7;
	return blockIndexInTexture + j;
}

int_t BlockCrops::getRenderType() { return 6; }

void BlockCrops::dropBlockAsItemWithChance(World *world, int_t i, int_t j, int_t k, int_t l, float f)
{
	dropBlockAsItemWithChance(world, i, j, k, l, f, 0);
}

void BlockCrops::dropBlockAsItemWithChance(World *world, int_t i, int_t j, int_t k, int_t l, float f, int_t fortune)
{
	BlockFlower::dropBlockAsItemWithChance(world, i, j, k, l, f, 0);
	if (world->multiplayerWorld) return;
	for (int_t i1 = 0; i1 < 3 + fortune; i1++)
	{
		if (world->rand.nextInt(15) <= l)
		{
			float f1 = 0.7f;
			float f2 = world->rand.nextFloat() * f1 + (1.0f - f1) * 0.5f;
			float f3 = world->rand.nextFloat() * f1 + (1.0f - f1) * 0.5f;
			float f4 = world->rand.nextFloat() * f1 + (1.0f - f1) * 0.5f;
			EntityItem *entityitem = new EntityItem(world, (float)i + f2, (float)j + f3, (float)k + f4, new ItemStack(Item::seeds));
			entityitem->delayBeforeCanPickup = 10;
			if (!world->entityJoinedWorld(entityitem))
				delete entityitem;
		}
	}
}

int_t BlockCrops::idDropped(int_t i, Random &random)
{
	if (i == 7) return Item::wheat->shiftedIndex;
	return -1;
}

int_t BlockCrops::quantityDropped(Random &random) { return 1; }
