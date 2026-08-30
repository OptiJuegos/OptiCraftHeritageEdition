#include "BlockSandStone.h"
#include "Material.h"

BlockSandStone::BlockSandStone(int_t i) : Block(i, 192, Material::rock)
{
}

int_t BlockSandStone::getBlockTextureFromSideAndMetadata(int_t side, int_t metadata)
{
    if (side == 1 || (side == 0 && (metadata == 1 || metadata == 2))) return 176;
    if (side == 0) return 208;
    if (metadata == 1) return 229;
    if (metadata == 2) return 230;
    return 192;
}

int_t BlockSandStone::getBlockTextureFromSide(int_t i)
{
    if (i == 1) return blockIndexInTexture - 16;
    if (i == 0) return blockIndexInTexture + 16;
    return blockIndexInTexture;
}

int_t BlockSandStone::damageDropped(int_t metadata)
{
    return metadata;
}
