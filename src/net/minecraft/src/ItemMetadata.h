#pragma once

#include "ItemBlock.h"

class Block;

// net.minecraft.src.ItemMetadata
class ItemMetadata : public ItemBlock
{
public:
    ItemMetadata(int id, Block *block);

    int getIconFromDamage(int damage) override;
    int getMetadata(int damage) override;

private:
    Block *blockObj;
};
