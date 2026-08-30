#include "RecipesArmor.h"

#include "Block.h"
#include "CraftingManager.h"
#include "Item.h"
#include "ItemStack.h"

RecipesArmor::RecipesArmor()
{
	recipePatterns[0][0] = "XXX";
	recipePatterns[0][1] = "X X";
	recipePatternLengths[0] = 2;
	recipePatterns[1][0] = "X X";
	recipePatterns[1][1] = "XXX";
	recipePatterns[1][2] = "XXX";
	recipePatternLengths[1] = 3;
	recipePatterns[2][0] = "XXX";
	recipePatterns[2][1] = "X X";
	recipePatterns[2][2] = "X X";
	recipePatternLengths[2] = 3;
	recipePatterns[3][0] = "X X";
	recipePatterns[3][1] = "X X";
	recipePatternLengths[3] = 2;

	recipeItems[0][0] = Item::leather;
	recipeItems[0][1] = Block::fire;
	recipeItems[0][2] = Item::ingotIron;
	recipeItems[0][3] = Item::diamond;
	recipeItems[0][4] = Item::ingotGold;
	recipeItems[1][0] = Item::helmetLeather;
	recipeItems[1][1] = Item::helmetChain;
	recipeItems[1][2] = Item::helmetSteel;
	recipeItems[1][3] = Item::helmetDiamond;
	recipeItems[1][4] = Item::helmetGold;
	recipeItems[2][0] = Item::plateLeather;
	recipeItems[2][1] = Item::plateChain;
	recipeItems[2][2] = Item::plateSteel;
	recipeItems[2][3] = Item::plateDiamond;
	recipeItems[2][4] = Item::plateGold;
	recipeItems[3][0] = Item::legsLeather;
	recipeItems[3][1] = Item::legsChain;
	recipeItems[3][2] = Item::legsSteel;
	recipeItems[3][3] = Item::legsDiamond;
	recipeItems[3][4] = Item::legsGold;
	recipeItems[4][0] = Item::bootsLeather;
	recipeItems[4][1] = Item::bootsChain;
	recipeItems[4][2] = Item::bootsSteel;
	recipeItems[4][3] = Item::bootsDiamond;
	recipeItems[4][4] = Item::bootsGold;
}

void RecipesArmor::addRecipes(CraftingManager *craftingmanager)
{
	for (int_t i = 0; i < 5; i++)
	{
		// recipeItems[0][1] is Block::fire — all other materials are Item*.
		void *material = recipeItems[0][i];
		bool isBlock = (i == 1);
		for (int_t j = 0; j < 4; j++)
		{
			Item *item = static_cast<Item *>(recipeItems[j + 1][i]);
			ItemStack *output = new ItemStack(item);
			switch (recipePatternLengths[j])
			{
				case 2:
					if (isBlock)
						craftingmanager->addRecipe(output, { recipePatterns[j][0], recipePatterns[j][1], 'X', static_cast<Block *>(material) });
					else
						craftingmanager->addRecipe(output, { recipePatterns[j][0], recipePatterns[j][1], 'X', static_cast<Item *>(material) });
					break;
				case 3:
					if (isBlock)
						craftingmanager->addRecipe(output, { recipePatterns[j][0], recipePatterns[j][1], recipePatterns[j][2], 'X', static_cast<Block *>(material) });
					else
						craftingmanager->addRecipe(output, { recipePatterns[j][0], recipePatterns[j][1], recipePatterns[j][2], 'X', static_cast<Item *>(material) });
					break;
				default:
					break;
			}
		}
	}
}
