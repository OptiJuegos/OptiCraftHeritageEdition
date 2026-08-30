#pragma once

#include "BlockBreakable.h"

// net.minecraft.src.BlockGlass
class BlockGlass : public BlockBreakable
{
public:
	BlockGlass(int_t i, int_t j, Material *material, bool flag);
	int_t quantityDropped(Random &random) override;
	int_t getRenderBlockPass() override;
	bool isOpaqueCube() override;
	bool renderAsNormalBlock() override;
	bool func_50074_q() override;
};
