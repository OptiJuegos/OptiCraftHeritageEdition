#include "ItemColored.h"

#include "Block.h"
#include "ItemStack.h"

ItemColored::ItemColored(int id, bool subtypes)
    : ItemBlock(id), blockRef(blockID >= 0 && blockID < Block::BLOCK_REGISTRY_SIZE ? Block::blocksList[blockID] : nullptr)
{
    if (subtypes)
    {
        setMaxDamage(0);
        setHasSubtypes(true);
    }
}

int ItemColored::getColorFromDamage(int damage, int)
{
    return blockRef != nullptr ? blockRef->getRenderColor(damage) : 0xffffff;
}

int ItemColored::getIconFromDamage(int damage)
{
    return blockRef != nullptr ? blockRef->getBlockTextureFromSideAndMetadata(0, damage) : 0;
}

int ItemColored::getMetadata(int damage)
{
    return damage;
}

ItemColored *ItemColored::setBlockNames(const std::vector<std::string> &names)
{
    blockNames = names;
    return this;
}

std::string ItemColored::getItemNameIS(ItemStack *itemstack)
{
    if (blockNames.empty() || itemstack == nullptr)
        return ItemBlock::getItemNameIS(itemstack);
    int damage = itemstack->getItemDamage();
    if (damage < 0 || damage >= (int)blockNames.size())
        return ItemBlock::getItemNameIS(itemstack);
    return ItemBlock::getItemNameIS(itemstack) + "." + blockNames[(std::size_t)damage];
}
