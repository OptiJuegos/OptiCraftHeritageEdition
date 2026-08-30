#pragma once

#include "Block.h"

// net.minecraft.src.BlockDirectional
class BlockDirectional : public Block
{
protected:
	BlockDirectional(int_t id, int_t texture, Material *material);
	BlockDirectional(int_t id, Material *material);

public:
	static int_t getDirection(int_t metadata);
};
