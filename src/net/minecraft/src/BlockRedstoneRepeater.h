#pragma once
#include "Block.h"

class EntityLiving;
class EntityPlayer;

// net.minecraft.src.BlockRedstoneRepeater
class BlockRedstoneRepeater : public Block
{
public:
    BlockRedstoneRepeater(int_t i, bool flag);
    bool renderAsNormalBlock() override;
    bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
    bool canBlockStay(World *world, int_t i, int_t j, int_t k) override;
    void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
    int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
    bool shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
    bool usesDefaultFaceCulling() const override { return false; }
    int_t getRenderType() override;
    int_t getBlockTextureFromSide(int_t i) override;
    bool isIndirectlyPoweringTo(World *world, int_t i, int_t j, int_t k, int_t l) override;
    bool isPoweringTo(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
    void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
    bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
    bool canProvidePower() override;
    void onBlockPlacedBy(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving) override;
    void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
    void onBlockDestroyedByPlayer(World *world, int_t i, int_t j, int_t k, int_t metadata) override;
    bool isOpaqueCube() override;
    int_t idDropped(int_t i, Random &random) override;
    void randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random) override;

    static const double isPowered[4];

private:
    bool getBlockTextureFromSideAndMetadata(World *world, int_t i, int_t j, int_t k, int_t l);
    static const int_t tickRate[4];
    bool isRepeaterPowered;
};
