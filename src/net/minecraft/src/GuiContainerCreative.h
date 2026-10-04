#pragma once

#include "GuiContainer.h"

class EntityPlayer;
class InventoryBasic;
class ItemStack;
class RenderItem;

// net.minecraft.src.GuiContainerCreative
class GuiContainerCreative : public GuiContainer
{
public:
    explicit GuiContainerCreative(EntityPlayer *player);

    void updateScreen() override;
    void initGui() override;
    void handleMouseInput() override;
    void mouseClicked(int_t mouseX, int_t mouseY, int_t button) override;
    void keyTyped(char_t c, int_t key) override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
    Slot *getControllerNavigationTarget(Slot *selected, int_t dirX, int_t dirY) override;
    bool scrollRows(int_t direction);

    static InventoryBasic *getInventory();
    void setCategory(int_t tabIndex);

protected:
    void handleMouseClick(Slot *slot, int_t slotId, int_t button, bool shift) override;
    void drawGuiContainerForegroundLayer() override;
    void drawGuiContainerBackgroundLayer(float_t partialTick) override;
    void actionPerformed(GuiButton *button) override;
#if defined(CTR_PLATFORM)
    // The category tabs sit 22 px above the 208 px body; reserving just
    // the tab height (not double) keeps the creative menu as large as the
    // panel allows while still showing the tabs.
    int_t containerPanelFitTopReserve() const override { return 22; }
#endif

private:
    void drawCategoryTabs(int_t guiLeft, int_t guiTop);
    void drawCreativeTabTooltip(const char *text, int_t x, int_t y);

    static InventoryBasic inventory;
    float_t currentScroll;
    bool isScrolling;
    bool wasClicking;
    static ItemStack *s_tabIcons[6];
    static RenderItem *creativeItemRenderer;
};
