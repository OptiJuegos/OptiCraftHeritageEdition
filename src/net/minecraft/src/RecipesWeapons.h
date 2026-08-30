#pragma once

#include "java/Type.h"

class CraftingManager;

// net.minecraft.src.RecipesWeapons
class RecipesWeapons
{
public:
	RecipesWeapons();

	void addRecipes(CraftingManager *craftingmanager);

private:
	const char *recipePatterns[1][3];
	int_t recipePatternLengths[1];
	void *recipeItems[2][5];
};
