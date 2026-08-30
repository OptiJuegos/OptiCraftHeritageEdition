#include "FurnaceRecipes.h"

#include "Block.h"
#include "Item.h"
#include "ItemStack.h"

FurnaceRecipes* FurnaceRecipes::smeltingBase = nullptr;

FurnaceRecipes::FurnaceRecipes()
{
    addSmelting(Block::oreIron->blockID,     new ItemStack(Item::ingotIron));
    addSmelting(Block::oreGold->blockID,     new ItemStack(Item::ingotGold));
    addSmelting(Block::oreDiamond->blockID,  new ItemStack(Item::diamond));
    addSmelting(Block::sand->blockID,        new ItemStack(Block::glass));
    addSmelting(Item::porkRaw->shiftedIndex,    new ItemStack(Item::porkCooked));
    addSmelting(Item::beefRaw->shiftedIndex,    new ItemStack(Item::beefCooked));
    addSmelting(Item::chickenRaw->shiftedIndex, new ItemStack(Item::chickenCooked));
    addSmelting(Item::fishRaw->shiftedIndex,    new ItemStack(Item::fishCooked));
    addSmelting(Block::cobblestone->blockID,    new ItemStack(Block::stone));
    addSmelting(Item::clay->shiftedIndex,       new ItemStack(Item::brick));
    addSmelting(Block::cactus->blockID,         new ItemStack(Item::dyePowder, 1, 2));
    addSmelting(Block::wood->blockID,           new ItemStack(Item::coal, 1, 1));
    addSmelting(Block::oreCoal->blockID,        new ItemStack(Item::coal));
    addSmelting(Block::oreRedstone->blockID,    new ItemStack(Item::redstone));
    addSmelting(Block::oreLapis->blockID,       new ItemStack(Item::dyePowder, 1, 4));
}

FurnaceRecipes* FurnaceRecipes::smelting()
{
    if (smeltingBase == nullptr)
        smeltingBase = new FurnaceRecipes();
    return smeltingBase;
}

void FurnaceRecipes::addSmelting(int_t itemId, ItemStack *output)
{
    smeltingOrder.add(itemId);
    smeltingList[itemId] = output;
}

ItemStack* FurnaceRecipes::getSmeltingResult(int_t itemId)
{
    auto it = smeltingList.find(itemId);
    if (it != smeltingList.end())
        return it->second;
    return nullptr;
}

std::map<int_t, ItemStack*>& FurnaceRecipes::getSmeltingList()
{
    return smeltingList;
}

std::vector<ItemStack*> FurnaceRecipes::getSmeltingValuesInJavaOrder() const
{
    std::vector<ItemStack*> values;
    values.reserve(smeltingList.size());
    for (int_t itemId : smeltingOrder.valuesInIterationOrder())
    {
        auto it = smeltingList.find(itemId);
        if (it != smeltingList.end())
            values.push_back(it->second);
    }
    return values;
}
