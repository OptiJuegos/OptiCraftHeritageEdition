#include "BlockBookshelf.h"
#include "Material.h"
#include "Item.h"

BlockBookshelf::BlockBookshelf(int_t i, int_t j) : Block(i, j, Material::wood)
{
}

int_t BlockBookshelf::getBlockTextureFromSide(int_t i)
{
    if (i <= 1) return 4;
    return blockIndexInTexture;
}

int_t BlockBookshelf::quantityDropped(Random &random)
{
    return 3;
}

int_t BlockBookshelf::idDropped(int_t metadata, Random &random, int_t fortune)
{
	return Item::book->shiftedIndex;
}
