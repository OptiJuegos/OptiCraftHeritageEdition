#pragma once

#include "java/Type.h"

class InventoryCrafting;
class ItemStack;

// net.minecraft.src.IRecipe
class IRecipe
{
public:
	virtual ~IRecipe() = default;
	virtual bool matches(InventoryCrafting *inventorycrafting) = 0;
	virtual ItemStack *getCraftingResult(InventoryCrafting *inventorycrafting) = 0;
	virtual int_t getRecipeSize() = 0;
	virtual ItemStack *getRecipeOutput() = 0;
};
