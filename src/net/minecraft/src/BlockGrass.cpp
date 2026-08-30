#include "BlockGrass.h"
#include "Material.h"
#include "IBlockAccess.h"
#include "WorldChunkManager.h"
#include "BiomeGenBase.h"
#include "ColorizerGrass.h"
#include "World.h"
#include "Block.h"
#include "Config.h"

BlockGrass::BlockGrass(int_t i) : Block(i, Material::grassMaterial)
{
	blockIndexInTexture = 3;
	setTickOnLoad(true);
}


int_t BlockGrass::getBlockTextureFromSideAndMetadata(int_t side, int_t)
{
	return side == 1 ? 0 : (side == 0 ? 2 : 3);
}

int_t BlockGrass::getBlockColor()
{
	return ColorizerGrass::getGrassColor(0.5, 1.0);
}

int_t BlockGrass::getRenderColor(int_t)
{
	return getBlockColor();
}

int_t BlockGrass::getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	if (l == 1) return 0;
	if (l == 0) return 2;
	Material *material = iblockaccess->getBlockMaterial(i, j + 1, k);
	return material != Material::snow && material != Material::builtSnow ? 3 : 68;
}

int_t BlockGrass::colorMultiplier(IBlockAccess *iblockaccess, int_t i, int_t, int_t k)
{
	if (!Config::isSmoothBiomes())
		return iblockaccess->getBiomeGenForCoords(i, k)->getBiomeGrassColor();

	int_t red = 0;
	int_t green = 0;
	int_t blue = 0;
	for (int_t dz = -1; dz <= 1; ++dz)
	{
		for (int_t dx = -1; dx <= 1; ++dx)
		{
			int_t color = iblockaccess->getBiomeGenForCoords(i + dx, k + dz)->getBiomeGrassColor();
			red += color >> 16 & 255;
			green += color >> 8 & 255;
			blue += color & 255;
		}
	}
	return (red / 9 & 255) << 16 | (green / 9 & 255) << 8 | blue / 9 & 255;
}

void BlockGrass::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	if (world->multiplayerWorld) return;
	if (world->getBlockLightValue(i, j + 1, k) < 4 && Block::lightOpacity[world->getBlockId(i, j + 1, k)] > 2)
	{
		world->setBlockWithNotify(i, j, k, Block::dirt->blockID);
	}
	else if (world->getBlockLightValue(i, j + 1, k) >= 9)
	{
		for (int_t attempt = 0; attempt < 4; ++attempt)
		{
			int_t x = i + random.nextInt(3) - 1;
			int_t y = j + random.nextInt(5) - 3;
			int_t z = k + random.nextInt(3) - 1;
			int_t above = world->getBlockId(x, y + 1, z);
			if (world->getBlockId(x, y, z) == Block::dirt->blockID &&
			    world->getBlockLightValue(x, y + 1, z) >= 4 &&
			    Block::lightOpacity[above] <= 2)
			{
				world->setBlockWithNotify(x, y, z, Block::grass->blockID);
			}
		}
	}
}

int_t BlockGrass::idDropped(int_t i, Random &random)
{
	return Block::dirt->idDropped(0, random);
}

int_t BlockGrass::idDropped(int_t i, Random &random, int_t fortune)
{
	(void)i;
	return Block::dirt->idDropped(0, random, fortune);
}
