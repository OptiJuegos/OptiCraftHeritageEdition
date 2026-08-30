#pragma once
#include "Block.h"
#include <vector>

// net.minecraft.src.BlockPistonExtension
class BlockPistonExtension : public Block
{
public:
    BlockPistonExtension(int_t i, int_t j);
    void getPistonExtensionState(int_t i);
    void getPistonExtensionFacing();
    void onBlockRemoval(World *world, int_t i, int_t j, int_t k) override;
    int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
    int_t getRenderType() override;
    bool isOpaqueCube() override;
    bool renderAsNormalBlock() override;
    bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
    bool canPlaceBlockOnSide(World *world, int_t i, int_t j, int_t k, int_t l) override;
    int_t quantityDropped(Random &random) override;
    void getCollidingBoundingBoxes(World *world, int_t i, int_t j, int_t k, AxisAlignedBB *axisalignedbb, std::vector<AxisAlignedBB*> &arraylist) override;
    void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
    void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
    static int_t getPistonExtensionType(int_t i);
    static int_t getDirectionMeta(int_t metadata);

private:
    int_t pistonState;
};
