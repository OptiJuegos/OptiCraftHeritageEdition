#include "BlockStoneBrick.h"

#include "Material.h"

BlockStoneBrick::BlockStoneBrick(int_t id) :
	Block(id, 54, Material::rock)
{
}

int_t BlockStoneBrick::getBlockTextureFromSideAndMetadata(int_t side, int_t metadata)
{
	switch (metadata)
	{
	case 1: return 100;
	case 2: return 101;
	case 3: return 213;
	default: return 54;
	}
}

int_t BlockStoneBrick::damageDropped(int_t metadata)
{
	return metadata;
}
