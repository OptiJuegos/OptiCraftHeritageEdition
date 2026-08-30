#include "BlockMobSpawner.h"
#include "Material.h"
#include "TileEntityMobSpawner.h"

BlockMobSpawner::BlockMobSpawner(int_t i, int_t j) : BlockContainer(i, j, Material::rock)
{
}

TileEntity *BlockMobSpawner::getBlockEntity()
{
    return new TileEntityMobSpawner();
}

int_t BlockMobSpawner::idDropped(int_t i, Random &random) { return 0; }
int_t BlockMobSpawner::quantityDropped(Random &random)    { return 0; }
bool  BlockMobSpawner::isOpaqueCube()                     { return false; }
