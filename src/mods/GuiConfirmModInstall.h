#pragma once

#include "GuiScreen.h"
#include "OchPackReader.h"
#include <string>

class GuiButton;

class GuiConfirmModInstall : public GuiScreen
{
public:
    GuiConfirmModInstall(GuiScreen *parent, const OchPackInfo &pack);

    void initGui() override;
    void actionPerformed(GuiButton *button) override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
    void keyTyped(char_t c, int_t key) override;
    void handleSpecializedMenuInput() override;
    bool allowsPlatformPointerInput() const override { return true; }

private:
    GuiScreen *parentScreen;
    OchPackInfo packInfo;
    std::string installedVersion;
    std::string statusMessage;
    bool isError = false;
};
