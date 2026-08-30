#pragma once

#include "java/Type.h"

class CraftingManager;

// net.minecraft.src.RecipesIngots
class RecipesIngots
{
public:
	RecipesIngots();

	void addRecipes(CraftingManager *craftingmanager);

private:
	void *recipeItems[4][2];
};
