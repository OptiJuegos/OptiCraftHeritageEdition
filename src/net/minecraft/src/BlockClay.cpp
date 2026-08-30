#include "BlockClay.h"

#include "Material.h"
#include "Item.h"

BlockClay::BlockClay(int_t i, int_t j) :
	Block(i, j, Material::clay)
{
}

int_t BlockClay::idDropped(int_t i, Random &random)
{
	return Item::clay->shiftedIndex;
}

int_t BlockClay::quantityDropped(Random &random)
{
	return 4;
}
