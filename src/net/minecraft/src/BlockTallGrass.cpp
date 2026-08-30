#include "BlockTallGrass.h"
#include "BiomeGenBase.h"
#include "IBlockAccess.h"
#include "WorldChunkManager.h"
#include "ColorizerGrass.h"
#include "ColorizerFoliage.h"
#include "Config.h"
#include "EntityPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "Material.h"
#include "StatList.h"
#include "World.h"
#include "java/Random.h"

BlockTallGrass::BlockTallGrass(int_t i, int_t j) : BlockFlower(i, j, Material::vine)
{
	float f = 0.4f;
	setBlockBounds(0.5f - f, 0.0f, 0.5f - f, 0.5f + f, 0.8f, 0.5f + f);
}

int_t BlockTallGrass::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
	if (j == 1) return blockIndexInTexture;
	if (j == 2) return blockIndexInTexture + 16 + 1;
	if (j == 0) return blockIndexInTexture + 16;
	return blockIndexInTexture;
}

int_t BlockTallGrass::getBlockColor()
{
	return ColorizerGrass::getGrassColor(0.5, 1.0);
}

int_t BlockTallGrass::getRenderColor(int_t metadata)
{
	return metadata == 0 ? 0xffffff : ColorizerFoliage::getFoliageColorBasic();
}

int_t BlockTallGrass::colorMultiplier(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	const int_t metadata = iblockaccess->getBlockMetadata(i, j, k);
	if (metadata == 0)
		return 0xffffff;
	if (!Config::isSmoothBiomes())
		return iblockaccess->getBiomeGenForCoords(i, k)->getBiomeGrassColor();

	int_t red = 0;
	int_t green = 0;
	int_t blue = 0;
	for (int_t dz = -1; dz <= 1; ++dz)
	{
		for (int_t dx = -1; dx <= 1; ++dx)
		{
			const int_t color = iblockaccess->getBiomeGenForCoords(i + dx, k + dz)->getBiomeGrassColor();
			red += color >> 16 & 255;
			green += color >> 8 & 255;
			blue += color & 255;
		}
	}
	return (red / 9 & 255) << 16 | (green / 9 & 255) << 8 | blue / 9 & 255;
}

int_t BlockTallGrass::idDropped(int_t i, Random &random)
{
	if (random.nextInt(8) == 0) return Item::seeds->shiftedIndex;
	return -1;
}

int_t BlockTallGrass::quantityDroppedWithBonus(int_t fortune, Random &random)
{
	return 1 + random.nextInt(fortune * 2 + 1);
}

void BlockTallGrass::harvestBlock(World *world, EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t metadata)
{
	ItemStack *equipped = entityplayer->getCurrentEquippedItem();
	if (!world->multiplayerWorld && equipped != nullptr && equipped->itemID == Item::shears->shiftedIndex)
	{
		if (blockID >= 0 && blockID < 256 && StatList::mineBlockStatArray[blockID] != nullptr)
		{
			entityplayer->addStat(StatList::mineBlockStatArray[blockID], 1);
		}
		dropBlockAsItem_do(world, i, j, k, new ItemStack(Block::tallGrass, 1, metadata));
		return;
	}

	BlockFlower::harvestBlock(world, entityplayer, i, j, k, metadata);
}
