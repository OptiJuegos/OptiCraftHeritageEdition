#pragma once

#include "Block.h"

// net.minecraft.src.BlockMushroomCap
class BlockMushroomCap : public Block
{
public:
    BlockMushroomCap(int_t id, Material *material, int_t texture, int_t mushroomType);

    int_t getBlockTextureFromSideAndMetadata(int_t side, int_t metadata) override;
    int_t quantityDropped(Random &random) override;
    int_t idDropped(int_t metadata, Random &random) override;

private:
    int_t mushroomType;
};
