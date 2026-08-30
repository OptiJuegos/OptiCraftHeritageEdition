#pragma once
#include "Block.h"
#include <vector>

class EntityPlayer;
class ChunkCoordinates;

// net.minecraft.src.BlockBed
class BlockBed : public Block
{
public:
    BlockBed(int_t i);
    bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
    int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
    int_t getRenderType() override;
    bool renderAsNormalBlock() override;
    bool isOpaqueCube() override;
    void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
    void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
    int_t idDropped(int_t i, Random &random) override;
    void dropBlockAsItemWithChance(World *world, int_t i, int_t j, int_t k, int_t l, float f) override;
    int_t getMobilityFlag() override;

    static int_t getDirectionFromMetadata(int_t i);
    static bool isBlockFootOfBed(int_t i);
    static bool isBedOccupied(int_t i);
    static void setBedOccupied(World *world, int_t i, int_t j, int_t k, bool flag);
    static ChunkCoordinates *getNearestEmptyChunkCoordinates(World *world, int_t i, int_t j, int_t k, int_t l);

    static const int_t headBlockToFootBlockMap[4][2];

private:
    void setBounds();
};
