#pragma once

#include "IRecipe.h"
#include <vector>

class ItemStack;

// net.minecraft.src.ShapedRecipes
class ShapedRecipes : public IRecipe
{
public:
    ShapedRecipes(int_t width, int_t height, ItemStack **items, ItemStack *output);
    ~ShapedRecipes();

    ItemStack* getRecipeOutput() override;
    bool matches(InventoryCrafting *inventorycrafting) override;
    ItemStack* getCraftingResult(InventoryCrafting *inventorycrafting) override;
    int_t getRecipeSize() override;

    const int_t recipeOutputItemID;

    // Read-only view of the pattern, for a recipe-list crafting menu.
    int_t getWidth() const { return recipeWidth; }
    int_t getHeight() const { return recipeHeight; }
    ItemStack *getPatternItem(int_t index) const { return recipeItems[index]; }

private:
    bool checkMatch(InventoryCrafting *inventorycrafting, int_t offX, int_t offY, bool mirror);

    int_t recipeWidth;
    int_t recipeHeight;
    ItemStack **recipeItems;
    ItemStack *recipeOutput;
};
