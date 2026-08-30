#pragma once

#include "BlockFlower.h"

// net.minecraft.src.BlockLilyPad
class BlockLilyPad : public BlockFlower
{
public:
    BlockLilyPad(int_t id, int_t texture);

    int_t getRenderType() override;
    AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t x, int_t y, int_t z) override;
    int_t getBlockColor() override;
    int_t getRenderColor(int_t metadata) override;
    int_t colorMultiplier(IBlockAccess *blockAccess, int_t x, int_t y, int_t z) override;
    bool canBlockStay(World *world, int_t x, int_t y, int_t z) override;

protected:
    bool canThisPlantGrowOnThisBlockID(int_t blockId) override;
};
