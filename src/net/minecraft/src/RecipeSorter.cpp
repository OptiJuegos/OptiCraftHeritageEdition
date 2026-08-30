#include "RecipeSorter.h"

#include "IRecipe.h"
#include "ShapedRecipes.h"
#include "ShapelessRecipes.h"

RecipeSorter::RecipeSorter(CraftingManager *craftingmanager) :
	craftingManager(craftingmanager)
{
}

int_t RecipeSorter::compareRecipes(IRecipe *irecipe, IRecipe *irecipe1)
{
	if (dynamic_cast<ShapelessRecipes *>(irecipe) != nullptr && dynamic_cast<ShapedRecipes *>(irecipe1) != nullptr)
	{
		return 1;
	}
	if (dynamic_cast<ShapelessRecipes *>(irecipe1) != nullptr && dynamic_cast<ShapedRecipes *>(irecipe) != nullptr)
	{
		return -1;
	}
	if (irecipe1->getRecipeSize() < irecipe->getRecipeSize())
	{
		return -1;
	}
	return irecipe1->getRecipeSize() <= irecipe->getRecipeSize() ? 0 : 1;
}

int_t RecipeSorter::compare(IRecipe *obj, IRecipe *obj1)
{
	return compareRecipes(obj, obj1);
}

bool RecipeSorter::operator()(IRecipe *irecipe, IRecipe *irecipe1)
{
	return compareRecipes(irecipe, irecipe1) < 0;
}
