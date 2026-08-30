#pragma once

#include "GuiScreen.h"

class Container;
class Slot;
class RenderItem;
class InventoryPlayer;

// net.minecraft.src.GuiContainer
class GuiContainer : public GuiScreen
{
public:
	GuiContainer(Container *container, bool ownsContainer = false);
	virtual ~GuiContainer();

	void initGui() override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	virtual void drawGuiContainerForegroundLayer();
	virtual void drawGuiContainerBackgroundLayer(float_t partialTick) = 0;

private:
	void drawSlotInventory(Slot *slot);

public:
	// Public for the console D-pad slot navigation (ContainerSlotNavigator),
	// which has to ask which slot the simulated cursor is over before it can
	// step to the next one. Sharing the click path's own hit test is what keeps
	// the two from disagreeing about where a slot ends.
	Slot *getSlotAtPosition(int_t mouseX, int_t mouseY);

private:
	bool getIsMouseOverSlot(Slot *slot, int_t mouseX, int_t mouseY);

protected:
	void mouseClicked(int_t x, int_t y, int_t button) override;
	virtual void handleMouseClick(Slot *slot, int_t slotId, int_t button, bool shift);
	void mouseMovedOrUp(int_t x, int_t y, int_t button) override;
	void keyTyped(char_t c, int_t key) override;

public:
	void onGuiClosed() override;
	bool doesGuiPauseGame() override;
	void updateScreen() override;

private:
	static RenderItem *itemRenderer;

protected:
	int_t xSize;
	int_t ySize;
	int_t guiLeft;
	int_t guiTop;

public:
	Container *inventorySlots;

private:
	bool ownsInventorySlots;
};
