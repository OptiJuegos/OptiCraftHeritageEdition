#include "platform/Log.h"
#include "CraftingManager.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "Block.h"
#include "IRecipe.h"
#include "InventoryCrafting.h"
#include "Item.h"
#include "ItemStack.h"
#include "RecipeSorter.h"
#include "RecipesArmor.h"
#include "RecipesCrafting.h"
#include "RecipesDyes.h"
#include "RecipesFood.h"
#include "RecipesIngots.h"
#include "RecipesTools.h"
#include "RecipesWeapons.h"
#include "ShapedRecipes.h"
#include "ShapelessRecipes.h"

CraftingManager *CraftingManager::instance = nullptr;

CraftingManager *CraftingManager::getInstance()
{
	if (instance == nullptr)
		instance = new CraftingManager();
	return instance;
}

// Builds a one-element ItemStack from any RecipeArg payload — mirrors the Java
// instanceof ladder inside addRecipe / addShapelessRecipe.
static ItemStack *argToStack(const RecipeArg &arg)
{
	switch (arg.kind)
	{
		case RecipeArg::KIND_ITEM:  return new ItemStack(arg.item);
		case RecipeArg::KIND_BLOCK: return new ItemStack(arg.block, 1, -1);
		case RecipeArg::KIND_STACK: return arg.stack;
		default:                    return nullptr;
	}
}

CraftingManager::CraftingManager()
{
	RecipesTools().addRecipes(this);
	RecipesWeapons().addRecipes(this);
	RecipesIngots().addRecipes(this);
	RecipesFood().addRecipes(this);
	RecipesCrafting().addRecipes(this);
	RecipesArmor().addRecipes(this);
	RecipesDyes().addRecipes(this);

	addRecipe(new ItemStack(Item::paper, 3),                  { "###", '#', Item::reed });
	addRecipe(new ItemStack(Item::book, 1),                   { "#", "#", "#", '#', Item::paper });
	addRecipe(new ItemStack(Block::fence, 2),                 { "###", "###", '#', Item::stick });
	addRecipe(new ItemStack(Block::netherFence, 6),           { "###", "###", '#', Block::netherBrick });
	addRecipe(new ItemStack(Block::fenceGate, 1),             { "#W#", "#W#", '#', Item::stick, 'W', Block::planks });
	addRecipe(new ItemStack(Block::jukebox, 1),               { "###", "#X#", "###", '#', Block::planks, 'X', Item::diamond });
	addRecipe(new ItemStack(Block::musicBlock, 1),            { "###", "#X#", "###", '#', Block::planks, 'X', Item::redstone });
	addRecipe(new ItemStack(Block::bookShelf, 1),             { "###", "XXX", "###", '#', Block::planks, 'X', Item::book });
	addRecipe(new ItemStack(Block::blockSnow, 1),             { "##", "##", '#', Item::snowball });
	addRecipe(new ItemStack(Block::blockClay, 1),             { "##", "##", '#', Item::clay });
	addRecipe(new ItemStack(Block::brick, 1),                 { "##", "##", '#', Item::brick });
	addRecipe(new ItemStack(Block::glowStone, 1),             { "##", "##", '#', Item::lightStoneDust });
	addRecipe(new ItemStack(Block::cloth, 1),                 { "##", "##", '#', Item::silk });
	addRecipe(new ItemStack(Block::tnt, 1),                   { "X#X", "#X#", "X#X", 'X', Item::gunpowder, '#', Block::sand });
	addRecipe(new ItemStack(Block::stairSingle, 6, 3),        { "###", '#', Block::cobblestone });
	addRecipe(new ItemStack(Block::stairSingle, 6, 0),        { "###", '#', Block::stone });
	addRecipe(new ItemStack(Block::stairSingle, 6, 1),        { "###", '#', Block::sandStone });
	addRecipe(new ItemStack(Block::stairSingle, 6, 2),        { "###", '#', Block::planks });
	addRecipe(new ItemStack(Block::stairSingle, 6, 4),        { "###", '#', Block::brick });
	addRecipe(new ItemStack(Block::stairSingle, 6, 5),        { "###", '#', Block::stoneBrick });
	addRecipe(new ItemStack(Block::ladder, 3),                { "# #", "###", "# #", '#', Item::stick });
	addRecipe(new ItemStack(Item::doorWood, 1),               { "##", "##", "##", '#', Block::planks });
	addRecipe(new ItemStack(Block::trapdoor, 2),              { "###", "###", '#', Block::planks });
	addRecipe(new ItemStack(Item::doorSteel, 1),              { "##", "##", "##", '#', Item::ingotIron });
	addRecipe(new ItemStack(Item::sign, 1),                   { "###", "###", " X ", '#', Block::planks, 'X', Item::stick });
	addRecipe(new ItemStack(Item::cake, 1),                   { "AAA", "BEB", "CCC", 'A', Item::bucketMilk, 'B', Item::sugar, 'C', Item::wheat, 'E', Item::egg });
	addRecipe(new ItemStack(Item::sugar, 1),                  { "#", '#', Item::reed });
	addRecipe(new ItemStack(Block::planks, 4, 0),             { "#", '#', new ItemStack(Block::wood, 1, 0) });
	addRecipe(new ItemStack(Block::planks, 4, 1),             { "#", '#', new ItemStack(Block::wood, 1, 1) });
	addRecipe(new ItemStack(Block::planks, 4, 2),             { "#", '#', new ItemStack(Block::wood, 1, 2) });
	addRecipe(new ItemStack(Block::planks, 4, 3),             { "#", '#', new ItemStack(Block::wood, 1, 3) });
	addRecipe(new ItemStack(Item::stick, 4),                  { "#", "#", '#', Block::planks });
	addRecipe(new ItemStack(Block::torchWood, 4),             { "X", "#", 'X', Item::coal, '#', Item::stick });
	addRecipe(new ItemStack(Block::torchWood, 4),             { "X", "#", 'X', new ItemStack(Item::coal, 1, 1), '#', Item::stick });
	addRecipe(new ItemStack(Item::bowlEmpty, 4),              { "# #", " # ", '#', Block::planks });
	addRecipe(new ItemStack(Item::glassBottle, 3),            { "# #", " # ", '#', Block::glass });
	addRecipe(new ItemStack(Block::rail, 16),                 { "X X", "X#X", "X X", 'X', Item::ingotIron, '#', Item::stick });
	addRecipe(new ItemStack(Block::railPowered, 6),           { "X X", "X#X", "XRX", 'X', Item::ingotGold, 'R', Item::redstone, '#', Item::stick });
	addRecipe(new ItemStack(Block::railDetector, 6),          { "X X", "X#X", "XRX", 'X', Item::ingotIron, 'R', Item::redstone, '#', Block::pressurePlateStone });
	addRecipe(new ItemStack(Item::minecartEmpty, 1),          { "# #", "###", '#', Item::ingotIron });
	addRecipe(new ItemStack(Item::cauldron, 1),               { "# #", "# #", "###", '#', Item::ingotIron });
	addRecipe(new ItemStack(Item::brewingStand, 1),           { " B ", "###", '#', Block::cobblestone, 'B', Item::blazeRod });
	addRecipe(new ItemStack(Block::pumpkinLantern, 1),        { "A", "B", 'A', Block::pumpkin, 'B', Block::torchWood });
	addRecipe(new ItemStack(Item::minecartCrate, 1),          { "A", "B", 'A', Block::chest, 'B', Item::minecartEmpty });
	addRecipe(new ItemStack(Item::minecartPowered, 1),        { "A", "B", 'A', Block::stoneOvenIdle, 'B', Item::minecartEmpty });
	addRecipe(new ItemStack(Item::boat, 1),                   { "# #", "###", '#', Block::planks });
	addRecipe(new ItemStack(Item::bucketEmpty, 1),            { "# #", " # ", '#', Item::ingotIron });
	addRecipe(new ItemStack(Item::flintAndSteel, 1),          { "A ", " B", 'A', Item::ingotIron, 'B', Item::flint });
	addRecipe(new ItemStack(Item::bread, 1),                  { "###", '#', Item::wheat });
	addRecipe(new ItemStack(Block::stairCompactPlanks, 4),    { "#  ", "## ", "###", '#', Block::planks });
	addRecipe(new ItemStack(Item::fishingRod, 1),             { "  #", " #X", "# X", '#', Item::stick, 'X', Item::silk });
	addRecipe(new ItemStack(Block::stairCompactCobblestone, 4), { "#  ", "## ", "###", '#', Block::cobblestone });
	addRecipe(new ItemStack(Block::stairsBrick, 4),           { "#  ", "## ", "###", '#', Block::brick });
	addRecipe(new ItemStack(Block::stairsStoneBrickSmooth, 4), { "#  ", "## ", "###", '#', Block::stoneBrick });
	addRecipe(new ItemStack(Block::stairsNetherBrick, 4),     { "#  ", "## ", "###", '#', Block::netherBrick });
	addRecipe(new ItemStack(Item::painting, 1),               { "###", "#X#", "###", '#', Item::stick, 'X', Block::cloth });
	addRecipe(new ItemStack(Item::appleGold, 1),              { "###", "#X#", "###", '#', Item::goldNugget, 'X', Item::appleRed });
	addRecipe(new ItemStack(Block::lever, 1),                 { "X", "#", '#', Block::cobblestone, 'X', Item::stick });
	addRecipe(new ItemStack(Block::torchRedstoneActive, 1),   { "X", "#", '#', Item::stick, 'X', Item::redstone });
	addRecipe(new ItemStack(Item::redstoneRepeater, 1),       { "#X#", "III", '#', Block::torchRedstoneActive, 'X', Item::redstone, 'I', Block::stone });
	addRecipe(new ItemStack(Item::pocketSundial, 1),          { " # ", "#X#", " # ", '#', Item::ingotGold, 'X', Item::redstone });
	addRecipe(new ItemStack(Item::compass, 1),                { " # ", "#X#", " # ", '#', Item::ingotIron, 'X', Item::redstone });
	addRecipe(new ItemStack(Item::mapItem, 1),                { "###", "#X#", "###", '#', Item::paper, 'X', Item::compass });
	addRecipe(new ItemStack(Block::button, 1),                { "#", "#", '#', Block::stone });
	addRecipe(new ItemStack(Block::pressurePlateStone, 1),    { "##", '#', Block::stone });
	addRecipe(new ItemStack(Block::pressurePlatePlanks, 1),   { "##", '#', Block::planks });
	addRecipe(new ItemStack(Block::dispenser, 1),             { "###", "#X#", "#R#", '#', Block::cobblestone, 'X', Item::bow, 'R', Item::redstone });
	addRecipe(new ItemStack(Block::pistonBase, 1),            { "TTT", "#X#", "#R#", '#', Block::cobblestone, 'X', Item::ingotIron, 'R', Item::redstone, 'T', Block::planks });
	addRecipe(new ItemStack(Block::pistonStickyBase, 1),      { "S", "P", 'S', Item::slimeBall, 'P', Block::pistonBase });
	addRecipe(new ItemStack(Item::bed, 1),                    { "###", "XXX", '#', Block::cloth, 'X', Block::planks });
	addRecipe(new ItemStack(Block::enchantmentTable, 1),      { " B ", "D#D", "###", '#', Block::obsidian, 'B', Item::book, 'D', Item::diamond });
	addShapelessRecipe(new ItemStack(Item::eyeOfEnder, 1),         { Item::enderPearl, Item::blazePowder });
	addShapelessRecipe(new ItemStack(Item::fireballCharge, 3),     { Item::gunpowder, Item::blazePowder, Item::coal });
	addShapelessRecipe(new ItemStack(Item::fireballCharge, 3),     { Item::gunpowder, Item::blazePowder, new ItemStack(Item::coal, 1, 1) });

	std::stable_sort(recipes.begin(), recipes.end(), RecipeSorter(this));
	MC_LOG_INFO("recipe", "%u recipes\n", static_cast<unsigned>(recipes.size()));
}

void CraftingManager::addRecipe(ItemStack *itemstack, std::initializer_list<RecipeArg> args)
{
	std::string shape;
	int_t width = 0;
	int_t height = 0;

	auto it = args.begin();
	const auto end = args.end();

	// Java reads consecutive String entries (the shape rows) until it hits a
	// Character. In our model strings are KIND_STRING, characters are KIND_CHAR.
	while (it != end && it->kind == RecipeArg::KIND_STRING)
	{
		const char *row = it->str;
		height++;
		width = static_cast<int_t>(std::char_traits<char>::length(row));
		shape += row;
		++it;
	}

	// Remainder is a sequence of (Character, Item|Block|ItemStack) pairs.
	std::unordered_map<char, ItemStack *> symbols;
	while (it != end)
	{
		if (it->kind != RecipeArg::KIND_CHAR)
			throw std::runtime_error("Invalid shaped recipe: expected Character key");
		char key = it->ch;
		++it;
		if (it == end)
			throw std::runtime_error("Invalid shaped recipe: missing value for key");
		symbols[key] = argToStack(*it);
		++it;
	}

	ItemStack **items = new ItemStack *[static_cast<size_t>(width) * static_cast<size_t>(height)]();
	for (int_t i = 0; i < width * height; i++)
	{
		char c = shape[static_cast<size_t>(i)];
		auto found = symbols.find(c);
		if (found != symbols.end() && found->second != nullptr)
			items[i] = found->second->copy();
		else
			items[i] = nullptr;
	}

	recipes.push_back(new ShapedRecipes(width, height, items, itemstack));
}

void CraftingManager::addShapelessRecipe(ItemStack *itemstack, std::initializer_list<RecipeArg> args)
{
	std::vector<ItemStack *> ingredients;
	ingredients.reserve(args.size());
	for (const RecipeArg &arg : args)
	{
		switch (arg.kind)
		{
			case RecipeArg::KIND_STACK: ingredients.push_back(arg.stack->copy()); break;
			case RecipeArg::KIND_ITEM:  ingredients.push_back(new ItemStack(arg.item)); break;
			case RecipeArg::KIND_BLOCK: ingredients.push_back(new ItemStack(arg.block)); break;
			default:
				throw std::runtime_error("Invalid shapeless recipy!");
		}
	}
	recipes.push_back(new ShapelessRecipes(itemstack, ingredients));
}

ItemStack *CraftingManager::findMatchingRecipe(InventoryCrafting *inventorycrafting)
{
	int_t occupiedSlots = 0;
	ItemStack *first = nullptr;
	ItemStack *second = nullptr;

	for (int_t slot = 0; slot < inventorycrafting->getSizeInventory(); ++slot)
	{
		ItemStack *stack = inventorycrafting->getStackInSlot(slot);
		if (stack == nullptr)
		{
			continue;
		}

		if (occupiedSlots == 0)
		{
			first = stack;
		}
		else if (occupiedSlots == 1)
		{
			second = stack;
		}
		++occupiedSlots;
	}

	if (occupiedSlots == 2 && first != nullptr && second != nullptr &&
	    first->itemID == second->itemID && first->stackSize == 1 && second->stackSize == 1)
	{
		Item *item = first->itemID >= 0 && first->itemID < Item::ITEM_LIST_SIZE ? Item::itemsList[first->itemID] : nullptr;
		if (item != nullptr && item->getMaxDamage() > 0)
		{
			const int_t remainingFirst = item->getMaxDamage() - first->getItemDamageForDisplay();
			const int_t remainingSecond = item->getMaxDamage() - second->getItemDamageForDisplay();
			const int_t repairedDurability = remainingFirst + remainingSecond + item->getMaxDamage() * 10 / 100;
			int_t damage = item->getMaxDamage() - repairedDurability;
			if (damage < 0)
			{
				damage = 0;
			}
			return new ItemStack(first->itemID, 1, damage);
		}
	}

	for (IRecipe *irecipe : recipes)
	{
		if (irecipe->matches(inventorycrafting))
		{
			return irecipe->getCraftingResult(inventorycrafting);
		}
	}
	return nullptr;
}

const std::vector<IRecipe *> &CraftingManager::getRecipeList()
{
	return recipes;
}


void CraftingManager::cleanup()
{
    if (instance == nullptr)
        return;

    for (IRecipe *recipe : instance->recipes)
    {
        delete recipe;
    }
    instance->recipes.clear();
    delete instance;
    instance = nullptr;
}
