#pragma once
#include "BlockContainer.h"
#include "java/Random.h"

class EntityPlayer;
class EntityLiving;
class TileEntityDispenser;

// net.minecraft.src.BlockDispenser
class BlockDispenser : public BlockContainer
{
public:
    BlockDispenser(int_t i);
    int_t tickRate() override;
    int_t idDropped(int_t i, Random &random) override;
    void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
    int_t getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
    int_t getBlockTextureFromSide(int_t i) override;
    bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
    void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
    void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
    void onBlockPlacedBy(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving) override;
    void onBlockRemoval(World *world, int_t i, int_t j, int_t k) override;

protected:
    TileEntity *getBlockEntity() override;

private:
    void setDispenserDefaultDirection(World *world, int_t i, int_t j, int_t k);
    void dispenseItem(World *world, int_t i, int_t j, int_t k, Random &random);
    Random random;
};
