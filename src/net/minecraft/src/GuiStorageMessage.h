#pragma once
#include "legacy/LegacyOptionsScreen.h"

class GuiStorageMessage : public LegacyOptionsScreen
{
public:
    GuiStorageMessage(GuiScreen *parent, GameSettings *settings, const std::string &message);
    void initGui() override;
    void drawScreen(int_t x, int_t y, float_t tick) override;
protected:
    void actionPerformed(GuiButton *button) override;
    void returnToParent() override;
private:
    std::string message;
};
