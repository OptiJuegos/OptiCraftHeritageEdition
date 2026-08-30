#pragma once
#include "Block.h"

class EntityPlayer;
class Vec3D;
class MovingObjectPosition;

// net.minecraft.src.BlockDoor
class BlockDoor : public Block
{
public:
	bool getBlocksMovement(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
    BlockDoor(int_t i, Material *material);
    int_t getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
    int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
    bool isOpaqueCube() override;
    bool renderAsNormalBlock() override;
    int_t getRenderType() override;
    AxisAlignedBB *getSelectedBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
    AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
    void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
    int_t getDoorOrientation(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k);
    int_t getFullMetadata(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k);
    void setDoorRotation(int_t i);
    void onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
    bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
    void onPoweredBlockChange(World *world, int_t i, int_t j, int_t k, bool flag);
    void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
    int_t idDropped(int_t i, Random &random) override;
    MovingObjectPosition *collisionRayTrace(World *world, int_t i, int_t j, int_t k, Vec3D *vec3d, Vec3D *vec3d1) override;
    int_t getState(int_t i);
    bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
    static bool isOpen(int_t i);
    int_t getMobilityFlag() override;
};
