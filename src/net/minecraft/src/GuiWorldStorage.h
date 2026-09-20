#pragma once
#ifdef PS2_PLATFORM
#include "legacy/LegacyOptionsScreen.h"

class GuiWorldStorage : public LegacyOptionsScreen
{
public:
    GuiWorldStorage(GuiScreen *parent, GameSettings *settings);
    void initGui() override;
    void drawScreen(int_t x, int_t y, float_t tick) override;
protected:
    void actionPerformed(GuiButton *button) override;
};
#endif
