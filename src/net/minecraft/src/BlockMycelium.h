#pragma once

#include "Block.h"

// net.minecraft.src.BlockMycelium
class BlockMycelium : public Block
{
public:
    explicit BlockMycelium(int_t id);

    int_t getBlockTextureFromSideAndMetadata(int_t side, int_t metadata) override;
    int_t getBlockTexture(IBlockAccess *blockAccess, int_t x, int_t y, int_t z, int_t side) override;
    void updateTick(World *world, int_t x, int_t y, int_t z, Random &random) override;
    void randomDisplayTick(World *world, int_t x, int_t y, int_t z, Random &random) override;
    int_t idDropped(int_t metadata, Random &random) override;
    int_t idDropped(int_t metadata, Random &random, int_t fortune) override;
};
