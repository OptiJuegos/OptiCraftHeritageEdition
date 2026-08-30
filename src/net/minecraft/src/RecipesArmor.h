#pragma once

#include "java/Type.h"

class CraftingManager;

// net.minecraft.src.RecipesArmor
class RecipesArmor
{
public:
	RecipesArmor();

	void addRecipes(CraftingManager *craftingmanager);

private:
	const char *recipePatterns[4][3];
	int_t recipePatternLengths[4];
	void *recipeItems[5][5];
};
