#include "ShapelessRecipes.h"

#include "ItemStack.h"
#include "InventoryCrafting.h"

ShapelessRecipes::ShapelessRecipes(ItemStack *output, std::vector<ItemStack*> items) :
    recipeOutput(output),
    recipeItems(items)
{
}

ShapelessRecipes::~ShapelessRecipes()
{
    delete recipeOutput;
    for (ItemStack *stack : recipeItems)
    {
        delete stack;
    }
}

ItemStack* ShapelessRecipes::getRecipeOutput()
{
    return recipeOutput;
}

bool ShapelessRecipes::matches(InventoryCrafting *inventorycrafting)
{
    std::vector<ItemStack*> remaining(recipeItems);

    for (int_t i = 0; i < 3; i++)
    {
        for (int_t j = 0; j < 3; j++)
        {
            ItemStack *present = inventorycrafting->getStackAt(j, i);
            if (present == nullptr)
                continue;

            bool found = false;
            for (auto it = remaining.begin(); it != remaining.end(); ++it)
            {
                ItemStack *required = *it;
                if (present->itemID == required->itemID &&
                    (required->getItemDamage() == -1 || present->getItemDamage() == required->getItemDamage()))
                {
                    found = true;
                    remaining.erase(it);
                    break;
                }
            }

            if (!found)
                return false;
        }
    }

    return remaining.empty();
}

ItemStack* ShapelessRecipes::getCraftingResult(InventoryCrafting *inventorycrafting)
{
    return recipeOutput->copy();
}

int_t ShapelessRecipes::getRecipeSize()
{
    return (int_t)recipeItems.size();
}
