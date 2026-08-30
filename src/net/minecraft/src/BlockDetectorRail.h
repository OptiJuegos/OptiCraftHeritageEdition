#pragma once
#include "BlockRail.h"

class Entity;

// net.minecraft.src.BlockDetectorRail
class BlockDetectorRail : public BlockRail
{
public:
    BlockDetectorRail(int_t i, int_t j);
    int_t tickRate() override;
    bool canProvidePower() override;
    void onEntityCollidedWithBlock(World *world, int_t i, int_t j, int_t k, Entity *entity) override;
    void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
    bool isPoweringTo(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
    bool isIndirectlyPoweringTo(World *world, int_t i, int_t j, int_t k, int_t l) override;

private:
    void setStateIfMinecartInteractsWithRail(World *world, int_t i, int_t j, int_t k, int_t l);
};
