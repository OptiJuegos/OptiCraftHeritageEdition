#pragma once

#include "GuiScreen.h"

class Container;
class Slot;
class RenderItem;
class InventoryPlayer;
class EntityPlayer;

// net.minecraft.src.GuiContainer
class GuiContainer : public GuiScreen
{
	friend class TooManyItems;
public:
	GuiContainer(Container *container, bool ownsContainer = false, EntityPlayer *player = nullptr);
	virtual ~GuiContainer();

	EntityPlayer *getContainerPlayer() const;
	void setContainerPlayer(EntityPlayer *player);
	int getOwnerPlayerIndex() const override;

	void initGui() override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	virtual void drawGuiContainerForegroundLayer();
	virtual void drawGuiContainerBackgroundLayer(float_t partialTick) = 0;
#if defined(CTR_PLATFORM)
	// Dual-screen fit (see GuiContainer.cpp): screen chrome that lives
	// ABOVE the xSize/ySize body -- the creative menu's category tabs --
	// reserves this much vertical room in the fit so it stays on the panel.
	virtual int_t containerPanelFitTopReserve() const { return 0; }
#endif

private:
	void drawSlotInventory(Slot *slot);

public:
	// Public for the console D-pad slot navigation (ContainerSlotNavigator),
	// which has to ask which slot the simulated cursor is over before it can
	// step to the next one. Sharing the click path's own hit test is what keeps
	// the two from disagreeing about where a slot ends.
	Slot *getSlotAtPosition(int_t mouseX, int_t mouseY);
	// A container can keep controller navigation inside a specialized slot grid.
	// Returning nullptr uses the normal geometric navigation.
	virtual Slot *getControllerNavigationTarget(Slot *selected, int_t dirX, int_t dirY);
	virtual void handleMouseClick(Slot *slot, int_t slotId, int_t button, bool shift);

private:
	bool getIsMouseOverSlot(Slot *slot, int_t mouseX, int_t mouseY);

protected:
	void mouseClicked(int_t x, int_t y, int_t button) override;
	void mouseMovedOrUp(int_t x, int_t y, int_t button) override;
	void keyTyped(char_t c, int_t key) override;

public:
	void onGuiClosed() override;
	bool doesGuiPauseGame() override;
	void updateScreen() override;
	bool allowsPlatformPointerInput() const override { return true; }

private:
	static RenderItem *itemRenderer;

protected:
	int_t xSize;
	int_t ySize;
	int_t guiLeft;
	int_t guiTop;
	EntityPlayer *m_containerPlayer;

public:
	Container *inventorySlots;

private:
	bool ownsInventorySlots;
};
