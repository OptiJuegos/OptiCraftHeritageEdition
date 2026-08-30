#pragma once

#include "Block.h"

class IBlockAccess;

// net.minecraft.src.BlockStep
class BlockStep : public Block
{
public:
	BlockStep(int_t i, bool flag);
	void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
	void setBlockBoundsForItemRender() override;
	void getCollidingBoundingBoxes(World *world, int_t i, int_t j, int_t k, AxisAlignedBB *axisalignedbb, std::vector<AxisAlignedBB *> &arraylist) override;
	int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
	int_t getBlockTextureFromSide(int_t i) override;
	bool isOpaqueCube() override;
	void onBlockPlaced(World *world, int_t i, int_t j, int_t k, int_t side) override;
	int_t idDropped(int_t i, Random &random) override;
	int_t quantityDropped(Random &random) override;
	bool renderAsNormalBlock() override;
	bool shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
	bool usesDefaultFaceCulling() const override { return false; }

	static const jstring blockStepTypes[6];

protected:
	int_t damageDropped(int_t i) override;
	ItemStack *createStackedBlock(int_t metadata) override;

private:
	bool blockType;
};
