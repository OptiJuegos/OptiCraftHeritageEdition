#pragma once

#include "Block.h"

class World;
class EntityPlayer;

// net.minecraft.src.BlockWorkbench
class BlockWorkbench : public Block
{
public:
	BlockWorkbench(int_t i);
	int_t getBlockTextureFromSide(int_t i) override;
	bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
};
