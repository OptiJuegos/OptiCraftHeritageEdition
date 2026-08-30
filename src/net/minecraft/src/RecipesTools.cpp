#include "RecipesTools.h"

#include "Block.h"
#include "CraftingManager.h"
#include "Item.h"
#include "ItemStack.h"

RecipesTools::RecipesTools()
{
	recipePatterns[0][0] = "XXX";
	recipePatterns[0][1] = " # ";
	recipePatterns[0][2] = " # ";
	recipePatternLengths[0] = 3;
	recipePatterns[1][0] = "X";
	recipePatterns[1][1] = "#";
	recipePatterns[1][2] = "#";
	recipePatternLengths[1] = 3;
	recipePatterns[2][0] = "XX";
	recipePatterns[2][1] = "X#";
	recipePatterns[2][2] = " #";
	recipePatternLengths[2] = 3;
	recipePatterns[3][0] = "XX";
	recipePatterns[3][1] = " #";
	recipePatterns[3][2] = " #";
	recipePatternLengths[3] = 3;

	recipeItems[0][0] = Block::planks;
	recipeItems[0][1] = Block::cobblestone;
	recipeItems[0][2] = Item::ingotIron;
	recipeItems[0][3] = Item::diamond;
	recipeItems[0][4] = Item::ingotGold;
	recipeItems[1][0] = Item::pickaxeWood;
	recipeItems[1][1] = Item::pickaxeStone;
	recipeItems[1][2] = Item::pickaxeSteel;
	recipeItems[1][3] = Item::pickaxeDiamond;
	recipeItems[1][4] = Item::pickaxeGold;
	recipeItems[2][0] = Item::shovelWood;
	recipeItems[2][1] = Item::shovelStone;
	recipeItems[2][2] = Item::shovelSteel;
	recipeItems[2][3] = Item::shovelDiamond;
	recipeItems[2][4] = Item::shovelGold;
	recipeItems[3][0] = Item::axeWood;
	recipeItems[3][1] = Item::axeStone;
	recipeItems[3][2] = Item::axeSteel;
	recipeItems[3][3] = Item::axeDiamond;
	recipeItems[3][4] = Item::axeGold;
	recipeItems[4][0] = Item::hoeWood;
	recipeItems[4][1] = Item::hoeStone;
	recipeItems[4][2] = Item::hoeSteel;
	recipeItems[4][3] = Item::hoeDiamond;
	recipeItems[4][4] = Item::hoeGold;
}

// Mirrors the Java pattern of building Object[] from a row count + material.
static void addPatternRecipe(CraftingManager *cm, ItemStack *output, const char *const *rows, int_t rowCount, void *material, bool materialIsBlock)
{
	// Translate each row + the (X, material) and (#, stick) pairs.
	switch (rowCount)
	{
		case 2:
			if (materialIsBlock)
				cm->addRecipe(output, { rows[0], rows[1], 'X', static_cast<Block *>(material), '#', Item::stick });
			else
				cm->addRecipe(output, { rows[0], rows[1], 'X', static_cast<Item *>(material), '#', Item::stick });
			break;
		case 3:
			if (materialIsBlock)
				cm->addRecipe(output, { rows[0], rows[1], rows[2], 'X', static_cast<Block *>(material), '#', Item::stick });
			else
				cm->addRecipe(output, { rows[0], rows[1], rows[2], 'X', static_cast<Item *>(material), '#', Item::stick });
			break;
		default:
			break;
	}
}

void RecipesTools::addRecipes(CraftingManager *craftingmanager)
{
	for (int_t i = 0; i < 5; i++)
	{
		// recipeItems[0] mixes Block::planks/cobblestone (Block*) with Item::ingot* / diamond (Item*).
		void *material = recipeItems[0][i];
		bool isBlock = (i <= 1); // planks, cobblestone are blocks; rest are items
		for (int_t j = 0; j < 4; j++)
		{
			Item *item = static_cast<Item *>(recipeItems[j + 1][i]);
			addPatternRecipe(craftingmanager, new ItemStack(item),
			                 recipePatterns[j], recipePatternLengths[j],
			                 material, isBlock);
		}
	}

	craftingmanager->addRecipe(new ItemStack(Item::shears),
	                           { " #", "# ", '#', Item::ingotIron });
}
