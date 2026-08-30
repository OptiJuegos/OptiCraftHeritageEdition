#include "RecipesWeapons.h"

#include "Block.h"
#include "CraftingManager.h"
#include "Item.h"
#include "ItemStack.h"

RecipesWeapons::RecipesWeapons()
{
	recipePatterns[0][0] = "X";
	recipePatterns[0][1] = "X";
	recipePatterns[0][2] = "#";
	recipePatternLengths[0] = 3;

	recipeItems[0][0] = Block::planks;
	recipeItems[0][1] = Block::cobblestone;
	recipeItems[0][2] = Item::ingotIron;
	recipeItems[0][3] = Item::diamond;
	recipeItems[0][4] = Item::ingotGold;
	recipeItems[1][0] = Item::swordWood;
	recipeItems[1][1] = Item::swordStone;
	recipeItems[1][2] = Item::swordSteel;
	recipeItems[1][3] = Item::swordDiamond;
	recipeItems[1][4] = Item::swordGold;
}

void RecipesWeapons::addRecipes(CraftingManager *craftingmanager)
{
	for (int_t i = 0; i < 5; i++)
	{
		// Same plank/cobblestone-are-blocks layout as RecipesTools.
		void *material = recipeItems[0][i];
		bool isBlock = (i <= 1);
		Item *item = static_cast<Item *>(recipeItems[1][i]);
		if (isBlock)
			craftingmanager->addRecipe(new ItemStack(item),
			                           { recipePatterns[0][0], recipePatterns[0][1], recipePatterns[0][2],
			                             'X', static_cast<Block *>(material), '#', Item::stick });
		else
			craftingmanager->addRecipe(new ItemStack(item),
			                           { recipePatterns[0][0], recipePatterns[0][1], recipePatterns[0][2],
			                             'X', static_cast<Item *>(material), '#', Item::stick });
	}

	craftingmanager->addRecipe(new ItemStack(Item::bow, 1),
	                           { " #X", "# X", " #X", 'X', Item::silk, '#', Item::stick });
	craftingmanager->addRecipe(new ItemStack(Item::arrow, 4),
	                           { "X", "#", "Y", 'Y', Item::feather, 'X', Item::flint, '#', Item::stick });
}
