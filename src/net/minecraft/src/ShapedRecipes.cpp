#include "ShapedRecipes.h"

#include "ItemStack.h"
#include "InventoryCrafting.h"

ShapedRecipes::ShapedRecipes(int_t width, int_t height, ItemStack **items, ItemStack *output) :
    recipeOutputItemID(output->itemID),
    recipeWidth(width),
    recipeHeight(height),
    recipeItems(items),
    recipeOutput(output)
{
}

ShapedRecipes::~ShapedRecipes()
{
    delete recipeOutput;
    int total = recipeWidth * recipeHeight;
    for (int i = 0; i < total; i++)
    {
        delete recipeItems[i];
    }
    delete[] recipeItems;
}

ItemStack* ShapedRecipes::getRecipeOutput()
{
    return recipeOutput;
}

bool ShapedRecipes::matches(InventoryCrafting *inventorycrafting)
{
    for (int_t i = 0; i <= 3 - recipeWidth; i++)
    {
        for (int_t j = 0; j <= 3 - recipeHeight; j++)
        {
            if (checkMatch(inventorycrafting, i, j, true))
                return true;
            if (checkMatch(inventorycrafting, i, j, false))
                return true;
        }
    }
    return false;
}

bool ShapedRecipes::checkMatch(InventoryCrafting *inventorycrafting, int_t offX, int_t offY, bool mirror)
{
    for (int_t k = 0; k < 3; k++)
    {
        for (int_t l = 0; l < 3; l++)
        {
            int_t i1 = k - offX;
            int_t j1 = l - offY;
            ItemStack *required = nullptr;

            if (i1 >= 0 && j1 >= 0 && i1 < recipeWidth && j1 < recipeHeight)
            {
                if (mirror)
                    required = recipeItems[(recipeWidth - i1 - 1) + j1 * recipeWidth];
                else
                    required = recipeItems[i1 + j1 * recipeWidth];
            }

            ItemStack *present = inventorycrafting->getStackAt(k, l);

            if (present == nullptr && required == nullptr)
                continue;
            if (present == nullptr || required == nullptr)
                return false;
            if (required->itemID != present->itemID)
                return false;
            if (required->getItemDamage() != -1 && required->getItemDamage() != present->getItemDamage())
                return false;
        }
    }
    return true;
}

ItemStack* ShapedRecipes::getCraftingResult(InventoryCrafting *inventorycrafting)
{
    return new ItemStack(recipeOutput->itemID, recipeOutput->stackSize, recipeOutput->getItemDamage());
}

int_t ShapedRecipes::getRecipeSize()
{
    return recipeWidth * recipeHeight;
}
