#include "BlockIce.h"
#include "Material.h"
#include "World.h"
#include "Block.h"
#include "EntityPlayer.h"
#include "EnumSkyBlock.h"
#include "IBlockAccess.h"

BlockIce::BlockIce(int_t i, int_t j) : BlockBreakable(i, j, Material::ice, false)
{
	slipperiness = 0.98f;
	setTickOnLoad(true);
}

int_t BlockIce::getRenderBlockPass() { return 1; }

bool BlockIce::shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	return BlockBreakable::shouldSideBeRendered(iblockaccess, i, j, k, 1 - l);
}

void BlockIce::harvestBlock(World *world, EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l)
{
	Block::harvestBlock(world, entityplayer, i, j, k, l);
	Material *material = world->getBlockMaterial(i, j - 1, k);
	if (material->getIsSolid() || material->getIsLiquid())
	{
		world->setBlockWithNotify(i, j, k, Block::waterMoving->blockID);
	}
}

int_t BlockIce::quantityDropped(Random &random) { return 0; }

void BlockIce::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	if (world->getSavedLightValue(EnumSkyBlock::Block, i, j, k) > 11 - Block::lightOpacity[blockID])
	{
		dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
		world->setBlockWithNotify(i, j, k, Block::waterStill->blockID);
	}
}

int_t BlockIce::getMobilityFlag() { return 0; }

ItemStack *BlockIce::createStackedBlock(int_t metadata)
{
	(void)metadata;
	return nullptr;
}
