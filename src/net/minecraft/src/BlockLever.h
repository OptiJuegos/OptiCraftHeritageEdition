#pragma once
#include "Block.h"

class EntityPlayer;

// net.minecraft.src.BlockLever
class BlockLever : public Block
{
public:
    BlockLever(int_t i, int_t j);
    AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
    bool isOpaqueCube() override;
    bool renderAsNormalBlock() override;
    int_t getRenderType() override;
    bool canPlaceBlockOnSide(World *world, int_t i, int_t j, int_t k, int_t l) override;
    bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
    void onBlockPlaced(World *world, int_t i, int_t j, int_t k, int_t l) override;
    void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
    void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
    void onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
    bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
    void onBlockRemoval(World *world, int_t i, int_t j, int_t k) override;
    bool isPoweringTo(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
    bool isIndirectlyPoweringTo(World *world, int_t i, int_t j, int_t k, int_t l) override;
    bool canProvidePower() override;

private:
    bool checkIfAttachedToBlock(World *world, int_t i, int_t j, int_t k);
};
