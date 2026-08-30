#pragma once

#include "GuiContainer.h"

class IInventory;

// net.minecraft.src.GuiChest
class GuiChest : public GuiContainer
{
public:
	GuiChest(IInventory *upper, IInventory *lower);

protected:
	void drawGuiContainerForegroundLayer() override;
	void drawGuiContainerBackgroundLayer(float_t partialTick) override;

private:
	IInventory *upperChestInventory;
	IInventory *lowerChestInventory;
	int_t inventoryRows;
};
