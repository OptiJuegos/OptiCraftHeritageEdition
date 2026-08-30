#pragma once

#include "Block.h"

class IBlockAccess;

// net.minecraft.src.BlockBreakable
class BlockBreakable : public Block
{
public:
	BlockBreakable(int_t i, int_t j, Material *material, bool flag);

	bool isOpaqueCube() override;
	bool shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
	bool usesDefaultFaceCulling() const override { return false; }

private:
	bool localFlag;
};
