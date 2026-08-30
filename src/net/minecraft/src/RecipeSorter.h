#pragma once

#include "java/Type.h"

class CraftingManager;
class IRecipe;

// net.minecraft.src.RecipeSorter
class RecipeSorter
{
public:
	RecipeSorter(CraftingManager *craftingmanager);

	int_t compareRecipes(IRecipe *irecipe, IRecipe *irecipe1);
	int_t compare(IRecipe *obj, IRecipe *obj1);
	bool operator()(IRecipe *irecipe, IRecipe *irecipe1);

private:
	CraftingManager *const craftingManager;
};
