#pragma once
#include "Block.h"

class Vec3D;
class MovingObjectPosition;
class RailLogic;

// net.minecraft.src.BlockRail
class BlockRail : public Block
{
public:
    static bool isRailBlockAt(World *world, int_t i, int_t j, int_t k);
    static bool isRailBlock(int_t i);

    BlockRail(int_t i, int_t j, bool flag);
    bool getIsPowered();
    AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
    bool isOpaqueCube() override;
    MovingObjectPosition *collisionRayTrace(World *world, int_t i, int_t j, int_t k, Vec3D *vec3d, Vec3D *vec3d1) override;
    void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
    int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
    bool renderAsNormalBlock() override;
    int_t getRenderType() override;
    int_t quantityDropped(Random &random) override;
    bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
    void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
    void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
    int_t getMobilityFlag() override;

    static bool isPoweredBlockRail(BlockRail *blockrail);

private:
    void getRailDirection(World *world, int_t i, int_t j, int_t k, bool flag);
    bool getRailDirection(World *world, int_t i, int_t j, int_t k, int_t l, bool flag, int_t i1);
    bool getRailMetadata(World *world, int_t i, int_t j, int_t k, bool flag, int_t l, int_t i1);
    bool isPowered;
};
