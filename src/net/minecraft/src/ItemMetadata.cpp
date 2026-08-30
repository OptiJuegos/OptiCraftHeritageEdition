#include "ItemMetadata.h"

#include "Block.h"

ItemMetadata::ItemMetadata(int id, Block *block)
    : ItemBlock(id), blockObj(block)
{
    setMaxDamage(0);
    setHasSubtypes(true);
}

int ItemMetadata::getIconFromDamage(int damage)
{
    return blockObj != nullptr ? blockObj->getBlockTextureFromSideAndMetadata(2, damage) : 0;
}

int ItemMetadata::getMetadata(int damage)
{
    return damage;
}
