#pragma once

#include "BlockBreakable.h"

class EntityPlayer;
class IBlockAccess;

// net.minecraft.src.BlockIce
class BlockIce : public BlockBreakable
{
public:
	BlockIce(int_t i, int_t j);
	int_t getRenderBlockPass() override;
	bool shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
	bool usesDefaultFaceCulling() const override { return false; }
	void harvestBlock(World *world, EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l) override;
	int_t quantityDropped(Random &random) override;
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
	int_t getMobilityFlag() override;

protected:
	ItemStack *createStackedBlock(int_t metadata) override;
};
