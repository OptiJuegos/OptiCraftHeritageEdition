#include "BlockMelon.h"

#include "Item.h"
#include "Material.h"

BlockMelon::BlockMelon(int_t id) :
	Block(id, Material::pumpkin)
{
	blockIndexInTexture = 136;
}

int_t BlockMelon::getBlockTextureFromSideAndMetadata(int_t side, int_t metadata)
{
	return side != 1 && side != 0 ? 136 : 137;
}

int_t BlockMelon::getBlockTextureFromSide(int_t side)
{
	return side != 1 && side != 0 ? 136 : 137;
}

int_t BlockMelon::idDropped(int_t metadata, Random &random, int_t fortune)
{
	return Item::melon != nullptr ? Item::melon->shiftedIndex : 0;
}

int_t BlockMelon::quantityDropped(Random &random)
{
	return 3 + random.nextInt(5);
}

int_t BlockMelon::quantityDroppedWithBonus(int_t fortune, Random &random)
{
	int_t amount = quantityDropped(random) + random.nextInt(1 + fortune);
	return amount > 9 ? 9 : amount;
}
