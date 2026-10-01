#pragma once

#include "IRecipe.h"
#include <vector>

class ItemStack;

// net.minecraft.src.ShapelessRecipes
class ShapelessRecipes : public IRecipe
{
public:
    ShapelessRecipes(ItemStack *output, std::vector<ItemStack*> items);
    ~ShapelessRecipes();

    ItemStack* getRecipeOutput() override;
    bool matches(InventoryCrafting *inventorycrafting) override;
    ItemStack* getCraftingResult(InventoryCrafting *inventorycrafting) override;
    int_t getRecipeSize() override;

    // Read-only view of the ingredients, for a recipe-list crafting menu.
    const std::vector<ItemStack*> &getIngredients() const { return recipeItems; }

private:
    ItemStack *recipeOutput;
    std::vector<ItemStack*> recipeItems;
};
