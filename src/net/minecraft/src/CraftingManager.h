#pragma once

#include <initializer_list>
#include <vector>

#include "java/Type.h"

class Block;
class InventoryCrafting;
class IRecipe;
class Item;
class ItemStack;

// net.minecraft.src.CraftingManager
//
// Java passes recipe entries as Object[] (mixing String, Character, Item, Block, ItemStack).
// Since C++ has no Object root, we model each Java-side Object with a small tagged
// variant — RecipeArg — that implicitly accepts every relevant payload type at the call
// site, so the recipe declarations in RecipesTools/Weapons/Ingots/Armor/Food/Crafting/Dyes
// remain a straight 1:1 translation of the Java source.
struct RecipeArg
{
	enum Kind
	{
		KIND_CHAR,
		KIND_STRING,
		KIND_ITEM,
		KIND_BLOCK,
		KIND_STACK
	};

	Kind kind;
	union
	{
		char        ch;
		const char *str;
		Item       *item;
		Block      *block;
		ItemStack  *stack;
	};

	RecipeArg(char c)            : kind(KIND_CHAR),   ch(c)    {}
	RecipeArg(const char *s)     : kind(KIND_STRING), str(s)   {}
	RecipeArg(Item *i)           : kind(KIND_ITEM),   item(i)  {}
	RecipeArg(Block *b)          : kind(KIND_BLOCK),  block(b) {}
	RecipeArg(ItemStack *st)     : kind(KIND_STACK),  stack(st){}
};

class CraftingManager
{
public:
	static CraftingManager *getInstance();
	static void cleanup();

	void addRecipe(ItemStack *itemstack, std::initializer_list<RecipeArg> args);
	void addShapelessRecipe(ItemStack *itemstack, std::initializer_list<RecipeArg> args);

	ItemStack *findMatchingRecipe(InventoryCrafting *inventorycrafting);
	const std::vector<IRecipe *> &getRecipeList();

private:
	CraftingManager();

	static CraftingManager *instance;
	std::vector<IRecipe *> recipes;
};
