#pragma once

#include "GuiScreen.h"
#include <string>

class GuiSlotMods;
class GuiButton;

class GuiMods : public GuiScreen
{
public:
    GuiMods(GuiScreen *parent);
    ~GuiMods() override;

    void initGui() override;
    void actionPerformed(GuiButton *button) override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
    void keyTyped(char_t c, int_t key) override;
    bool allowsPlatformPointerInput() const override { return true; }

    FontRenderer *getFont() { return fontRenderer; }

private:
    GuiScreen *parentScreen;
    GuiSlotMods *slotList;
    std::string screenTitle;
    friend class GuiSlotMods;
};
