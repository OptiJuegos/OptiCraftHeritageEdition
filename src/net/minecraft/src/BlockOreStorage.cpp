#include "BlockOreStorage.h"
#include "Material.h"

BlockOreStorage::BlockOreStorage(int_t i, int_t j) : Block(i, Material::iron)
{
    blockIndexInTexture = j;
}

int_t BlockOreStorage::getBlockTextureFromSide(int_t i)
{
    return blockIndexInTexture;
}
