#pragma once

#include "Block.h"

// net.minecraft.src.BlockEndPortalFrame
class BlockEndPortalFrame : public Block
{
public:
	explicit BlockEndPortalFrame(int_t id);

	int_t getBlockTextureFromSideAndMetadata(int_t side, int_t metadata) override;
	bool isOpaqueCube() override;
	int_t getRenderType() override;
	void setBlockBoundsForItemRender() override;
	void getCollidingBoundingBoxes(World *world, int_t x, int_t y, int_t z, AxisAlignedBB *mask, std::vector<AxisAlignedBB *> &list) override;
	int_t idDropped(int_t metadata, Random &random, int_t fortune) override;
	void onBlockPlacedBy(World *world, int_t x, int_t y, int_t z, EntityLiving *entityliving) override;

	static bool isEnderEyeInserted(int_t metadata);
};
