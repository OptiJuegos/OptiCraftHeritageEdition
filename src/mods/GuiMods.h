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
    void confirmClicked(bool confirmed, int_t id) override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
    void keyTyped(char_t c, int_t key) override;
    void handleSpecializedMenuInput() override;
    bool allowsPlatformPointerInput() const override { return true; }

    FontRenderer *getFont() { return fontRenderer; }

    int_t getSelectedModIndex() const { return selectedModIndex; }
    void setSelectedModIndex(int_t index);

private:
    GuiScreen *parentScreen;
    GuiSlotMods *slotList;
    std::string screenTitle;
    int_t selectedModIndex = -1;
    GuiButton *deleteButton = nullptr;

    friend class GuiSlotMods;
};
