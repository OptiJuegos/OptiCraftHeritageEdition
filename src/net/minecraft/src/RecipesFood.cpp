#include "RecipesFood.h"

#include "Block.h"
#include "BlockMushroom.h"
#include "CraftingManager.h"
#include "Item.h"
#include "ItemStack.h"

RecipesFood::RecipesFood()
{
}

void RecipesFood::addRecipes(CraftingManager *craftingmanager)
{
	craftingmanager->addShapelessRecipe(new ItemStack(Item::bowlSoup),
	                                    { Block::mushroomBrown, Block::mushroomRed, Item::bowlEmpty });
	craftingmanager->addRecipe(new ItemStack(Item::cookie, 8),
	                           { "#X#", 'X', new ItemStack(Item::dyePowder, 1, 3), '#', Item::wheat });
	craftingmanager->addRecipe(new ItemStack(Block::melon),
	                           { "MMM", "MMM", "MMM", 'M', Item::melon });
	craftingmanager->addRecipe(new ItemStack(Item::melonSeeds),
	                           { "M", 'M', Item::melon });
	craftingmanager->addRecipe(new ItemStack(Item::pumpkinSeeds, 4),
	                           { "M", 'M', Block::pumpkin });
	craftingmanager->addShapelessRecipe(new ItemStack(Item::fermentedSpiderEye),
	                                    { Item::spiderEye, Block::mushroomBrown, Item::sugar });
	craftingmanager->addShapelessRecipe(new ItemStack(Item::speckledMelon),
	                                    { Item::melon, Item::goldNugget });
	craftingmanager->addShapelessRecipe(new ItemStack(Item::blazePowder, 2),
	                                    { Item::blazeRod });
	craftingmanager->addShapelessRecipe(new ItemStack(Item::magmaCream),
	                                    { Item::blazePowder, Item::slimeBall });
}

