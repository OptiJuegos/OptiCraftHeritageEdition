#include "RecipesCrafting.h"

#include "Block.h"
#include "CraftingManager.h"
#include "Item.h"
#include "ItemStack.h"

RecipesCrafting::RecipesCrafting()
{
}

void RecipesCrafting::addRecipes(CraftingManager *craftingmanager)
{
	craftingmanager->addRecipe(new ItemStack(Block::chest),
	                           { "###", "# #", "###", '#', Block::planks });
	craftingmanager->addRecipe(new ItemStack(Block::stoneOvenIdle),
	                           { "###", "# #", "###", '#', Block::cobblestone });
	craftingmanager->addRecipe(new ItemStack(Block::workbench),
	                           { "##", "##", '#', Block::planks });
	craftingmanager->addRecipe(new ItemStack(Block::sandStone),
	                           { "##", "##", '#', Block::sand });
	craftingmanager->addRecipe(new ItemStack(Block::sandStone, 4, 2),
	                           { "##", "##", '#', Block::sandStone });
	craftingmanager->addRecipe(new ItemStack(Block::sandStone, 1, 1),
	                           { "#", "#", '#', new ItemStack(Block::stairSingle, 1, 1) });
	craftingmanager->addRecipe(new ItemStack(Block::stoneBrick, 4),
	                           { "##", "##", '#', Block::stone });
	craftingmanager->addRecipe(new ItemStack(Block::fenceIron, 16),
	                           { "###", "###", '#', Item::ingotIron });
	craftingmanager->addRecipe(new ItemStack(Block::thinGlass, 16),
	                           { "###", "###", '#', Block::glass });
	craftingmanager->addRecipe(new ItemStack(Block::redstoneLampIdle),
	                           { " R ", "RGR", " R ", 'R', Item::redstone, 'G', Block::glowStone });
}

