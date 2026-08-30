#pragma once

#include "java/Type.h"

class Container;
class ItemStack;

// net.minecraft.src.ICrafting
class ICrafting
{
public:
	virtual ~ICrafting() = default;
	virtual void sendSlotContents(Container *container, int_t slotIndex, ItemStack *itemstack) = 0;
	virtual void sendProgressBarUpdate(Container *container, int_t varToUpdate, int_t newValue) = 0;
};
