#pragma once

#include "BlockFluid.h"

// net.minecraft.src.BlockStationary
class BlockStationary : public BlockFluid
{
public:
	bool getBlocksMovement(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
	BlockStationary(int_t i, Material *material);
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;

private:
	void getLiquidMaterial(World *world, int_t i, int_t j, int_t k);
	bool getIsStationary(World *world, int_t i, int_t j, int_t k);
};
