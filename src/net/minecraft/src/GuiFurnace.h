#pragma once

#include "GuiContainer.h"

class InventoryPlayer;
class TileEntityFurnace;

// net.minecraft.src.GuiFurnace
class GuiFurnace : public GuiContainer
{
public:
	GuiFurnace(InventoryPlayer *player, TileEntityFurnace *furnace);

protected:
	void drawGuiContainerForegroundLayer() override;
	void drawGuiContainerBackgroundLayer(float_t partialTick) override;

private:
	TileEntityFurnace *furnaceInventory;
};
