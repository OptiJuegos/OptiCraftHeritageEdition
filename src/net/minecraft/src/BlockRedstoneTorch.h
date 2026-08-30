#pragma once
#include "BlockTorch.h"
#include "RedstoneUpdateInfo.h"
#include <vector>

// net.minecraft.src.BlockRedstoneTorch
class BlockRedstoneTorch : public BlockTorch
{
public:
    BlockRedstoneTorch(int_t i, int_t j, bool flag);
    int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
    int_t tickRate() override;
    void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
    void onBlockRemoval(World *world, int_t i, int_t j, int_t k) override;
    bool isPoweringTo(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
    void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
    void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
    bool isIndirectlyPoweringTo(World *world, int_t i, int_t j, int_t k, int_t l) override;
    int_t idDropped(int_t i, Random &random) override;
    bool canProvidePower() override;
    void randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random) override;

private:
    bool checkForBurnout(World *world, int_t i, int_t j, int_t k, bool flag);
    bool getFlowDecay(World *world, int_t i, int_t j, int_t k);
    bool torchActive;
    static std::vector<RedstoneUpdateInfo> torchUpdates;
};
