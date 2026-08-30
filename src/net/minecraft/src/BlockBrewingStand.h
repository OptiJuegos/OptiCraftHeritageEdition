#pragma once

#include "BlockContainer.h"
#include "java/Random.h"

class TileEntity;

// net.minecraft.src.BlockBrewingStand
class BlockBrewingStand : public BlockContainer
{
public:
    explicit BlockBrewingStand(int_t id);

    bool isOpaqueCube() override;
    int_t getRenderType() override;
    bool renderAsNormalBlock() override;
    void getCollidingBoundingBoxes(World *world, int_t x, int_t y, int_t z, AxisAlignedBB *mask,
                                   std::vector<AxisAlignedBB *> &boxes) override;
    void setBlockBoundsForItemRender() override;
    bool blockActivated(World *world, int_t x, int_t y, int_t z, EntityPlayer *player) override;
    void randomDisplayTick(World *world, int_t x, int_t y, int_t z, Random &random) override;
    void onBlockRemoval(World *world, int_t x, int_t y, int_t z) override;
    int_t idDropped(int_t metadata, Random &random) override;

protected:
    TileEntity *getBlockEntity() override;

private:
    Random random;
};
