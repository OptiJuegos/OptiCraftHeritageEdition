#include "BlockWood.h"

#include "Material.h"

BlockWood::BlockWood(int_t id) :
	Block(id, 4, Material::wood)
{
}

int_t BlockWood::getBlockTextureFromSideAndMetadata(int_t side, int_t metadata)
{
	switch (metadata)
	{
	case 1: return 198;
	case 2: return 214;
	case 3: return 199;
	default: return 4;
	}
}

int_t BlockWood::damageDropped(int_t metadata)
{
	return metadata;
}
