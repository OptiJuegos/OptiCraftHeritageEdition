#include "BlockLeaves.h"
#include "Config.h"
#include "Material.h"
#include "ColorizerFoliage.h"
#include "IBlockAccess.h"
#include "WorldChunkManager.h"
#include "BiomeGenBase.h"
#include "World.h"
#include "Block.h"
#include "EntityPlayer.h"
#include "ItemStack.h"
#include "Item.h"
#include "StatList.h"
#include "Entity.h"
#include "java/Arithmetic.h"

BlockLeaves::BlockLeaves(int_t i, int_t j) : BlockLeavesBase(i, j, Material::leaves, false)
{
	baseIndexInPNG = j;
	adjacentTreeBlocks = nullptr;
	setTickOnLoad(true);
}

BlockLeaves::~BlockLeaves()
{
	delete[] adjacentTreeBlocks;
}

int_t BlockLeaves::getBlockColor()
{
	return ColorizerFoliage::getFoliageColor(0.5, 1.0);
}

int_t BlockLeaves::getRenderColor(int_t i)
{
	if ((i & 3) == 1) return ColorizerFoliage::getFoliageColorPine();
	if ((i & 3) == 2) return ColorizerFoliage::getFoliageColorBirch();
	return ColorizerFoliage::getFoliageColorBasic();
}

int_t BlockLeaves::colorMultiplier(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	int_t metadata = iblockaccess->getBlockMetadata(i, j, k);
	if ((metadata & 3) == 1) return ColorizerFoliage::getFoliageColorPine();
	if ((metadata & 3) == 2) return ColorizerFoliage::getFoliageColorBirch();
	if (!Config::isSmoothBiomes())
		return iblockaccess->getBiomeGenForCoords(i, k)->getBiomeFoliageColor();
	int_t red = 0;
	int_t green = 0;
	int_t blue = 0;
	for (int_t dz = -1; dz <= 1; ++dz)
	{
		for (int_t dx = -1; dx <= 1; ++dx)
		{
			int_t color = iblockaccess->getBiomeGenForCoords(JavaArithmetic::intAdd(i, dx), JavaArithmetic::intAdd(k, dz))->getBiomeFoliageColor();
			red += color >> 16 & 255;
			green += color >> 8 & 255;
			blue += color & 255;
		}
	}
	return (red / 9 & 255) << 16 | (green / 9 & 255) << 8 | blue / 9 & 255;
}

void BlockLeaves::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
	int_t l  = 1;
	int_t i1 = l + 1;
	if (world->checkChunksExist(JavaArithmetic::intSub(i, i1), JavaArithmetic::intSub(j, i1), JavaArithmetic::intSub(k, i1), JavaArithmetic::intAdd(i, i1), JavaArithmetic::intAdd(j, i1), JavaArithmetic::intAdd(k, i1)))
	{
		for (int_t j1 = -l; j1 <= l; j1++)
		{
			for (int_t k1 = -l; k1 <= l; k1++)
			{
				for (int_t l1 = -l; l1 <= l; l1++)
				{
					int_t i2 = world->getBlockId(JavaArithmetic::intAdd(i, j1), JavaArithmetic::intAdd(j, k1), JavaArithmetic::intAdd(k, l1));
					if (i2 == Block::leaves->blockID)
					{
						int_t j2 = world->getBlockMetadata(JavaArithmetic::intAdd(i, j1), JavaArithmetic::intAdd(j, k1), JavaArithmetic::intAdd(k, l1));
						world->setBlockMetadata(JavaArithmetic::intAdd(i, j1), JavaArithmetic::intAdd(j, k1), JavaArithmetic::intAdd(k, l1), j2 | 8);
					}
				}
			}
		}
	}
}

void BlockLeaves::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	if (world->multiplayerWorld) return;
	int_t l = world->getBlockMetadata(i, j, k);
	if ((l & 8) != 0 && (l & 4) == 0)
	{
		byte_t byte0 = 4;
		int_t i1 = byte0 + 1;
		byte_t byte1 = 32;
		int_t j1 = byte1 * byte1;
		int_t k1 = byte1 / 2;
		if (adjacentTreeBlocks == nullptr)
		{
			adjacentTreeBlocks = new int_t[byte1 * byte1 * byte1]();
		}
		if (world->checkChunksExist(JavaArithmetic::intSub(i, i1), JavaArithmetic::intSub(j, i1), JavaArithmetic::intSub(k, i1), JavaArithmetic::intAdd(i, i1), JavaArithmetic::intAdd(j, i1), JavaArithmetic::intAdd(k, i1)))
		{
			for (int_t l1 = -byte0; l1 <= byte0; l1++)
			{
				for (int_t k2 = -byte0; k2 <= byte0; k2++)
				{
					for (int_t i3 = -byte0; i3 <= byte0; i3++)
					{
						int_t k3 = world->getBlockId(JavaArithmetic::intAdd(i, l1), JavaArithmetic::intAdd(j, k2), JavaArithmetic::intAdd(k, i3));
						if (k3 == Block::wood->blockID)
							adjacentTreeBlocks[(l1 + k1) * j1 + (k2 + k1) * byte1 + (i3 + k1)] = 0;
						else if (k3 == Block::leaves->blockID)
							adjacentTreeBlocks[(l1 + k1) * j1 + (k2 + k1) * byte1 + (i3 + k1)] = -2;
						else
							adjacentTreeBlocks[(l1 + k1) * j1 + (k2 + k1) * byte1 + (i3 + k1)] = -1;
					}
				}
			}
			for (int_t i2 = 1; i2 <= 4; i2++)
			{
				for (int_t l2 = -byte0; l2 <= byte0; l2++)
				{
					for (int_t j3 = -byte0; j3 <= byte0; j3++)
					{
						for (int_t l3 = -byte0; l3 <= byte0; l3++)
						{
							if (adjacentTreeBlocks[(l2 + k1) * j1 + (j3 + k1) * byte1 + (l3 + k1)] != i2 - 1) continue;
							if (adjacentTreeBlocks[((l2 + k1) - 1) * j1 + (j3 + k1) * byte1 + (l3 + k1)] == -2)
								adjacentTreeBlocks[((l2 + k1) - 1) * j1 + (j3 + k1) * byte1 + (l3 + k1)] = i2;
							if (adjacentTreeBlocks[(l2 + k1 + 1) * j1 + (j3 + k1) * byte1 + (l3 + k1)] == -2)
								adjacentTreeBlocks[(l2 + k1 + 1) * j1 + (j3 + k1) * byte1 + (l3 + k1)] = i2;
							if (adjacentTreeBlocks[(l2 + k1) * j1 + ((j3 + k1) - 1) * byte1 + (l3 + k1)] == -2)
								adjacentTreeBlocks[(l2 + k1) * j1 + ((j3 + k1) - 1) * byte1 + (l3 + k1)] = i2;
							if (adjacentTreeBlocks[(l2 + k1) * j1 + (j3 + k1 + 1) * byte1 + (l3 + k1)] == -2)
								adjacentTreeBlocks[(l2 + k1) * j1 + (j3 + k1 + 1) * byte1 + (l3 + k1)] = i2;
							if (adjacentTreeBlocks[(l2 + k1) * j1 + (j3 + k1) * byte1 + ((l3 + k1) - 1)] == -2)
								adjacentTreeBlocks[(l2 + k1) * j1 + (j3 + k1) * byte1 + ((l3 + k1) - 1)] = i2;
							if (adjacentTreeBlocks[(l2 + k1) * j1 + (j3 + k1) * byte1 + (l3 + k1 + 1)] == -2)
								adjacentTreeBlocks[(l2 + k1) * j1 + (j3 + k1) * byte1 + (l3 + k1 + 1)] = i2;
						}
					}
				}
			}
		}
		int_t j2 = adjacentTreeBlocks[k1 * j1 + k1 * byte1 + k1];
		if (j2 >= 0)
		{
			world->setBlockMetadata(i, j, k, l & -9);
		}
		else
		{
			removeLeaves(world, i, j, k);
		}
	}
}

void BlockLeaves::removeLeaves(World *world, int_t i, int_t j, int_t k)
{
	dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
	world->setBlockWithNotify(i, j, k, 0);
}

int_t BlockLeaves::quantityDropped(Random &random)
{
	return random.nextInt(20) != 0 ? 0 : 1;
}

int_t BlockLeaves::idDropped(int_t i, Random &random, int_t fortune)
{
	return Block::sapling->blockID;
}

void BlockLeaves::dropBlockAsItemWithChance(World *world, int_t i, int_t j, int_t k, int_t l, float f, int_t fortune)
{
	if (world->multiplayerWorld)
		return;
	int_t chance = (l & 3) == 3 ? 40 : 20;
	if (world->rand.nextInt(chance) == 0)
	{
		int_t droppedId = idDropped(l, world->rand, fortune);
		dropBlockAsItem_do(world, i, j, k, new ItemStack(droppedId, 1, damageDropped(l)));
	}
	if ((l & 3) == 0 && world->rand.nextInt(200) == 0)
		dropBlockAsItem_do(world, i, j, k, new ItemStack(Item::appleRed, 1, 0));
}

void BlockLeaves::harvestBlock(World *world, EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l)
{
	if (!world->multiplayerWorld && entityplayer->getCurrentEquippedItem() != nullptr &&
	    entityplayer->getCurrentEquippedItem()->itemID == Item::shears->shiftedIndex)
	{
		entityplayer->addStat(StatList::mineBlockStatArray[blockID], 1);
		dropBlockAsItem_do(world, i, j, k, new ItemStack(Block::leaves->blockID, 1, l & 3));
	}
	else
	{
		BlockLeavesBase::harvestBlock(world, entityplayer, i, j, k, l);
	}
}

int_t BlockLeaves::damageDropped(int_t i) { return i & 3; }

bool BlockLeaves::isOpaqueCube() { return !graphicsLevel; }

int_t BlockLeaves::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
	if ((j & 3) == 1) return blockIndexInTexture + 80;
	if ((j & 3) == 3) return blockIndexInTexture + 144;
	return blockIndexInTexture;
}

void BlockLeaves::setGraphicsLevel(bool flag)
{
	graphicsLevel = flag;
	blockIndexInTexture = baseIndexInPNG + (flag ? 0 : 1);
}

void BlockLeaves::onEntityWalking(World *world, int_t i, int_t j, int_t k, Entity *entity)
{
	BlockLeavesBase::onEntityWalking(world, i, j, k, entity);
}
