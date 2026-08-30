#pragma once

#include "BlockContainer.h"

// net.minecraft.src.BlockEndPortal
class BlockEndPortal : public BlockContainer
{
public:
    BlockEndPortal(int_t id, Material *material);

    void setBlockBoundsBasedOnState(IBlockAccess *access, int_t x, int_t y, int_t z) override;
    bool shouldSideBeRendered(IBlockAccess *access, int_t x, int_t y, int_t z, int_t side) override;
    void getCollidingBoundingBoxes(World *world, int_t x, int_t y, int_t z, AxisAlignedBB *mask,
                                   std::vector<AxisAlignedBB *> &boxes) override;
    bool isOpaqueCube() override;
    bool renderAsNormalBlock() override;
    int_t quantityDropped(Random &random) override;
    void onEntityCollidedWithBlock(World *world, int_t x, int_t y, int_t z, Entity *entity) override;
    void randomDisplayTick(World *world, int_t x, int_t y, int_t z, Random &random) override;
    int_t getRenderType() override;
    void onBlockAdded(World *world, int_t x, int_t y, int_t z) override;

    static bool bossDefeated;

protected:
    TileEntity *getBlockEntity() override;
};
