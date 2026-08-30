#include "RecipesIngots.h"

#include "Block.h"
#include "CraftingManager.h"
#include "Item.h"
#include "ItemStack.h"

RecipesIngots::RecipesIngots()
{
	recipeItems[0][0] = Block::blockGold;
	recipeItems[0][1] = new ItemStack(Item::ingotGold, 9);
	recipeItems[1][0] = Block::blockSteel;
	recipeItems[1][1] = new ItemStack(Item::ingotIron, 9);
	recipeItems[2][0] = Block::blockDiamond;
	recipeItems[2][1] = new ItemStack(Item::diamond, 9);
	recipeItems[3][0] = Block::blockLapis;
	recipeItems[3][1] = new ItemStack(Item::dyePowder, 9, 4);
}

void RecipesIngots::addRecipes(CraftingManager *craftingmanager)
{
	for (int_t i = 0; i < 4; i++)
	{
		Block *block = static_cast<Block *>(recipeItems[i][0]);
		ItemStack *itemstack = static_cast<ItemStack *>(recipeItems[i][1]);

		craftingmanager->addRecipe(new ItemStack(block),
		                           { "###", "###", "###", '#', itemstack });
		craftingmanager->addRecipe(itemstack,
		                           { "#", '#', block });
	}

	craftingmanager->addRecipe(new ItemStack(Item::ingotGold),
	                           { "###", "###", "###", '#', Item::goldNugget });
	craftingmanager->addRecipe(new ItemStack(Item::goldNugget, 9),
	                           { "#", '#', Item::ingotGold });
}
