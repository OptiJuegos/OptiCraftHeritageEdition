#pragma once
#include "Block.h"
#include "EnumMobType.h"

class Entity;

// net.minecraft.src.BlockPressurePlate
class BlockPressurePlate : public Block
{
public:
	bool getBlocksMovement(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
    BlockPressurePlate(int_t i, int_t j, EnumMobType enummobtype, Material *material);
    int_t tickRate() override;
    AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
    bool isOpaqueCube() override;
    bool renderAsNormalBlock() override;
    bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
    void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
    void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
    void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
    void onEntityCollidedWithBlock(World *world, int_t i, int_t j, int_t k, Entity *entity) override;
    void onBlockRemoval(World *world, int_t i, int_t j, int_t k) override;
    void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
    bool isPoweringTo(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
    bool isIndirectlyPoweringTo(World *world, int_t i, int_t j, int_t k, int_t l) override;
    bool canProvidePower() override;
    void setBlockBoundsForItemRender() override;
    int_t getMobilityFlag() override;

private:
    void setStateIfMobInteractsWithPlate(World *world, int_t i, int_t j, int_t k);
    EnumMobType triggerMobType;
};
