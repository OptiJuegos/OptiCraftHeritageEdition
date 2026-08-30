#include "BlockDirectional.h"

BlockDirectional::BlockDirectional(int_t id, int_t texture, Material *material) :
	Block(id, texture, material)
{
}

BlockDirectional::BlockDirectional(int_t id, Material *material) :
	Block(id, material)
{
}

int_t BlockDirectional::getDirection(int_t metadata)
{
	return metadata & 3;
}
