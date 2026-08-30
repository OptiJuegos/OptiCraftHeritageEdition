#include "BlockRedstoneOre.h"
#include "Material.h"
#include "World.h"
#include "Block.h"
#include "Item.h"
#include "ItemStack.h"
#include "EntityPlayer.h"
#include "Entity.h"

BlockRedstoneOre::BlockRedstoneOre(int_t i, int_t j, bool flag) : Block(i, j, Material::rock)
{
	if (flag) setTickOnLoad(true);
	shiftedIndex = flag;
}

int_t BlockRedstoneOre::tickRate() { return 30; }

void BlockRedstoneOre::onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
	getGlowingState(world, i, j, k);
	Block::onBlockClicked(world, i, j, k, entityplayer);
}

void BlockRedstoneOre::onEntityWalking(World *world, int_t i, int_t j, int_t k, Entity *entity)
{
	getGlowingState(world, i, j, k);
	Block::onEntityWalking(world, i, j, k, entity);
}

bool BlockRedstoneOre::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
	getGlowingState(world, i, j, k);
	return Block::blockActivated(world, i, j, k, entityplayer);
}

void BlockRedstoneOre::getGlowingState(World *world, int_t i, int_t j, int_t k)
{
	getLit(world, i, j, k);
	if (blockID == Block::oreRedstone->blockID)
	{
		world->setBlockWithNotify(i, j, k, Block::oreRedstoneGlowing->blockID);
	}
}

void BlockRedstoneOre::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	if (blockID == Block::oreRedstoneGlowing->blockID)
	{
		world->setBlockWithNotify(i, j, k, Block::oreRedstone->blockID);
	}
}

int_t BlockRedstoneOre::idDropped(int_t i, Random &random)
{
	return Item::redstone->shiftedIndex;
}

int_t BlockRedstoneOre::quantityDroppedWithBonus(int_t fortune, Random &random)
{
	return quantityDropped(random) + random.nextInt(fortune + 1);
}

int_t BlockRedstoneOre::quantityDropped(Random &random)
{
	return 4 + random.nextInt(2);
}

void BlockRedstoneOre::randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	if (shiftedIndex)
	{
		getLit(world, i, j, k);
	}
}

void BlockRedstoneOre::getLit(World *world, int_t i, int_t j, int_t k)
{
	Random &random = world->rand;
	double d = 0.0625;
	for (int_t l = 0; l < 6; l++)
	{
		double d1 = (float)i + random.nextFloat();
		double d2 = (float)j + random.nextFloat();
		double d3 = (float)k + random.nextFloat();
		if (l == 0 && !world->isBlockOpaqueCube(i, j + 1, k)) d2 = (double)(j + 1) + d;
		if (l == 1 && !world->isBlockOpaqueCube(i, j - 1, k)) d2 = (double)(j + 0) - d;
		if (l == 2 && !world->isBlockOpaqueCube(i, j, k + 1)) d3 = (double)(k + 1) + d;
		if (l == 3 && !world->isBlockOpaqueCube(i, j, k - 1)) d3 = (double)(k + 0) - d;
		if (l == 4 && !world->isBlockOpaqueCube(i + 1, j, k)) d1 = (double)(i + 1) + d;
		if (l == 5 && !world->isBlockOpaqueCube(i - 1, j, k)) d1 = (double)(i + 0) - d;
		if (d1 < (double)i || d1 > (double)(i + 1) ||
		    d2 < 0.0 || d2 > (double)(j + 1) ||
		    d3 < (double)k || d3 > (double)(k + 1))
		{
			world->spawnParticle("reddust", d1, d2, d3, 0.0, 0.0, 0.0);
		}
	}
}

ItemStack *BlockRedstoneOre::createStackedBlock(int_t metadata)
{
	return new ItemStack(Block::oreRedstone);
}
