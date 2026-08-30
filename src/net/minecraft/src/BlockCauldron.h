#pragma once

#include "Block.h"

// net.minecraft.src.BlockCauldron
class BlockCauldron : public Block
{
public:
    explicit BlockCauldron(int_t id);

    int_t getBlockTextureFromSideAndMetadata(int_t side, int_t metadata) override;
    void getCollidingBoundingBoxes(World *world, int_t x, int_t y, int_t z, AxisAlignedBB *mask,
                                   std::vector<AxisAlignedBB *> &boxes) override;
    void setBlockBoundsForItemRender() override;
    bool isOpaqueCube() override;
    int_t getRenderType() override;
    bool renderAsNormalBlock() override;
    bool blockActivated(World *world, int_t x, int_t y, int_t z, EntityPlayer *player) override;
    int_t idDropped(int_t metadata, Random &random) override;
};
