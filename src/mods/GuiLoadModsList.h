#pragma once

#include "GuiScreen.h"
#include "OchPackReader.h"
#include <vector>
#include <string>

class GuiSlotLoadMods;
class GuiButton;

class GuiLoadModsList : public GuiScreen
{
public:
    enum class Source
    {
        Device,
        USB
    };

    GuiLoadModsList(GuiScreen *parent, Source source);
    ~GuiLoadModsList() override;

    void initGui() override;
    void actionPerformed(GuiButton *button) override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
    void keyTyped(char_t c, int_t key) override;
    void updateScreen() override;
    void handleSpecializedMenuInput() override;
    bool allowsPlatformPointerInput() const override { return true; }

    int_t getSelectedPackIndex() const { return selectedIndex; }
    void setSelectedPackIndex(int_t index);
    void setPendingConfirmIndex(int_t index) { pendingConfirmIndex = index; }
    const std::vector<OchPackInfo> &getPacks() const { return availablePacks; }
    FontRenderer *getFont() { return fontRenderer; }

private:
    void scanPacks();

    GuiScreen *parentScreen;
    Source loadSource;
    GuiSlotLoadMods *slotList;
    std::vector<OchPackInfo> availablePacks;
    int_t selectedIndex = -1;
    int_t pendingConfirmIndex = -1;
    bool scanned_ = false;
    std::string screenTitle;
    std::string emptyMessage1;
    std::string emptyMessage2;
    std::vector<std::string> debugLogs;

    friend class GuiSlotLoadMods;
};
