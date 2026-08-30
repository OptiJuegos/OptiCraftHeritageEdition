#pragma once
#include "BlockContainer.h"
#include "java/Random.h"

class TileEntityFurnace;
class EntityPlayer;
class EntityLiving;
class TileEntity;

// net.minecraft.src.BlockFurnace
class BlockFurnace : public BlockContainer
{
public:
    BlockFurnace(int_t i, bool flag);
    int_t idDropped(int_t i, Random &random) override;
    void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
    int_t getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
    void randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
    int_t getBlockTextureFromSide(int_t i) override;
    bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
    static void updateFurnaceBlockState(bool flag, World *world, int_t i, int_t j, int_t k);
    void onBlockPlacedBy(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving) override;
    void onBlockRemoval(World *world, int_t i, int_t j, int_t k) override;

protected:
    TileEntity *getBlockEntity() override;

private:
    void setDefaultDirection(World *world, int_t i, int_t j, int_t k);
    Random furnaceRand;
    bool isActive;
    static bool keepFurnaceInventory;
};
