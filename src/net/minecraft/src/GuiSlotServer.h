#pragma once

#include "GuiSlot.h"

class GuiMultiplayer;

// net.minecraft.src.GuiSlotServer
class GuiSlotServer : public GuiSlot
{
public:
    explicit GuiSlotServer(GuiMultiplayer *parent);

protected:
    int_t getSize() override;
    void elementClicked(int_t index, bool doubleClicked) override;
    bool isSelected(int_t index) override;
    int_t getContentHeight() override;
    void drawBackground() override;
    void drawSlot(int_t index, int_t x, int_t y, int_t height, Tessellator *tessellator) override;

private:
    GuiMultiplayer *parentGui;
};
