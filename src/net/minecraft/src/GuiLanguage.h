#pragma once

#include "GuiScreen.h"

class GameSettings;
class GuiSlotLanguage;
class GuiSmallButton;

// net.minecraft.src.GuiLanguage
class GuiLanguage : public GuiScreen
{
public:
    GuiLanguage(GuiScreen *parent, GameSettings *settings);
    ~GuiLanguage() override;

    void initGui() override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
    void updateScreen() override;

protected:
    void actionPerformed(GuiButton *button) override;
    void mouseClicked(int_t x, int_t y, int_t button) override;
    void mouseMovedOrUp(int_t x, int_t y, int_t button) override;

private:
    friend class GuiSlotLanguage;
    GuiScreen *parentGui;
    int_t updateTimer;
    GuiSlotLanguage *languageList;
    GameSettings *settings;
    GuiSmallButton *doneButton;
};
