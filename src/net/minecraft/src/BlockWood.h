#pragma once

#include "Block.h"

// net.minecraft.src.BlockWood
class BlockWood : public Block
{
public:
	explicit BlockWood(int_t id);
	int_t getBlockTextureFromSideAndMetadata(int_t side, int_t metadata) override;

protected:
	int_t damageDropped(int_t metadata) override;
};
