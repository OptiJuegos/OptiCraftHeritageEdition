#include "BlockLockedChest.h"
#include "Material.h"
#include "IBlockAccess.h"
#include "World.h"
#include "Block.h"

BlockLockedChest::BlockLockedChest(int_t i) : Block(i, Material::wood)
{
	blockIndexInTexture = 26;
}

int_t BlockLockedChest::getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	if (l == 1) return blockIndexInTexture - 1;
	if (l == 0) return blockIndexInTexture - 1;
	int_t i1 = iblockaccess->getBlockId(i, j, k - 1);
	int_t j1 = iblockaccess->getBlockId(i, j, k + 1);
	int_t k1 = iblockaccess->getBlockId(i - 1, j, k);
	int_t l1 = iblockaccess->getBlockId(i + 1, j, k);
	byte_t byte0 = 3;
	if (Block::opaqueCubeLookup[i1] && !Block::opaqueCubeLookup[j1]) byte0 = 3;
	if (Block::opaqueCubeLookup[j1] && !Block::opaqueCubeLookup[i1]) byte0 = 2;
	if (Block::opaqueCubeLookup[k1] && !Block::opaqueCubeLookup[l1]) byte0 = 5;
	if (Block::opaqueCubeLookup[l1] && !Block::opaqueCubeLookup[k1]) byte0 = 4;
	return l != (int_t)byte0 ? blockIndexInTexture : blockIndexInTexture + 1;
}

int_t BlockLockedChest::getBlockTextureFromSide(int_t i)
{
	if (i == 1) return blockIndexInTexture - 1;
	if (i == 0) return blockIndexInTexture - 1;
	if (i == 3) return blockIndexInTexture + 1;
	return blockIndexInTexture;
}

bool BlockLockedChest::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
	return true;
}

void BlockLockedChest::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	world->setBlockWithNotify(i, j, k, 0);
}
