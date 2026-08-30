#pragma once
#include "BlockBreakable.h"

class Entity;

// net.minecraft.src.BlockPortal
class BlockPortal : public BlockBreakable
{
public:
    BlockPortal(int_t i, int_t j);
    AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
    void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
    bool isOpaqueCube() override;
    bool renderAsNormalBlock() override;
    bool tryToCreatePortal(World *world, int_t i, int_t j, int_t k);
    void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
    bool shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
    bool usesDefaultFaceCulling() const override { return false; }
    int_t quantityDropped(Random &random) override;
    int_t getRenderBlockPass() override;
    void onEntityCollidedWithBlock(World *world, int_t i, int_t j, int_t k, Entity *entity) override;
    void randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
};
