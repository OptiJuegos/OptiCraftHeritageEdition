#pragma once

#include "Block.h"

class World;

// net.minecraft.src.BlockSponge
class BlockSponge : public Block
{
public:
	BlockSponge(int_t i);
	void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
	void onBlockRemoval(World *world, int_t i, int_t j, int_t k) override;
};
