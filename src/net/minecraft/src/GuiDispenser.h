#pragma once

#include "GuiContainer.h"

class InventoryPlayer;
class TileEntityDispenser;

// net.minecraft.src.GuiDispenser
class GuiDispenser : public GuiContainer
{
public:
	GuiDispenser(InventoryPlayer *player, TileEntityDispenser *dispenser);

protected:
	void drawGuiContainerForegroundLayer() override;
	void drawGuiContainerBackgroundLayer(float_t partialTick) override;
};
