#pragma once

#include "Block.h"

// net.minecraft.src.BlockPane
class BlockPane : public Block
{
public:
	BlockPane(int_t id, int_t texture, int_t sideTexture, Material *material, bool canDropItself);

	int_t idDropped(int_t metadata, Random &random, int_t fortune) override;
	bool isOpaqueCube() override;
	bool renderAsNormalBlock() override;
	int_t getRenderType() override;
	bool shouldSideBeRendered(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z, int_t side) override;
	bool usesDefaultFaceCulling() const override { return false; }
	void getCollidingBoundingBoxes(World *world, int_t x, int_t y, int_t z, AxisAlignedBB *mask, std::vector<AxisAlignedBB *> &list) override;
	void setBlockBoundsForItemRender() override;
	void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z) override;
	int_t getSideTextureIndex() const;
	bool canThisPaneConnectToThisBlockID(int_t id) const;

private:
	int_t sideTextureIndex;
	const bool canDropItself;
};
