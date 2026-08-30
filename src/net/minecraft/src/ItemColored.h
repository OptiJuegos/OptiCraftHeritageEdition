#pragma once

#include <string>
#include <vector>

#include "ItemBlock.h"

class Block;
class ItemStack;

// net.minecraft.src.ItemColored
class ItemColored : public ItemBlock
{
public:
    ItemColored(int id, bool hasSubtypes);

    int getColorFromDamage(int damage, int renderPass) override;
    int getIconFromDamage(int damage) override;
    int getMetadata(int damage) override;
    ItemColored *setBlockNames(const std::vector<std::string> &names);
    std::string getItemNameIS(ItemStack *itemstack) override;

protected:
    Block *blockRef;
    std::vector<std::string> blockNames;
};
