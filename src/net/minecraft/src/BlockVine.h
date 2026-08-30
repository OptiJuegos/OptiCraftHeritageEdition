#pragma once

#include "Block.h"

// net.minecraft.src.BlockVine
class BlockVine : public Block
{
public:
    explicit BlockVine(int_t id);

    void setBlockBoundsForItemRender() override;
    int_t getRenderType() override;
    bool isOpaqueCube() override;
    bool renderAsNormalBlock() override;
    void setBlockBoundsBasedOnState(IBlockAccess *blockAccess, int_t x, int_t y, int_t z) override;
    AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t x, int_t y, int_t z) override;
    bool canPlaceBlockOnSide(World *world, int_t x, int_t y, int_t z, int_t side) override;
    int_t getBlockColor() override;
    int_t getRenderColor(int_t metadata) override;
    int_t colorMultiplier(IBlockAccess *blockAccess, int_t x, int_t y, int_t z) override;
    void onNeighborBlockChange(World *world, int_t x, int_t y, int_t z, int_t neighborId) override;
    void updateTick(World *world, int_t x, int_t y, int_t z, Random &random) override;
    void onBlockPlaced(World *world, int_t x, int_t y, int_t z, int_t side) override;
    int_t idDropped(int_t metadata, Random &random) override;
    int_t quantityDropped(Random &random) override;
    void harvestBlock(World *world, EntityPlayer *player, int_t x, int_t y, int_t z, int_t metadata) override;

private:
    bool canBePlacedOn(int_t blockId) const;
    bool canVineStay(World *world, int_t x, int_t y, int_t z);
};
