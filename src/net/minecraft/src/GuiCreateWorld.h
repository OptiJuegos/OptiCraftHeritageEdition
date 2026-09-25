#pragma once

#include "GuiScreen.h"
#include <string>

class GuiTextField;
class GuiButton;
class ISaveFormat;

// net.minecraft.src.GuiCreateWorld
class GuiCreateWorld : public GuiScreen
{
public:
    GuiCreateWorld(GuiScreen *parent);
    ~GuiCreateWorld() override;

    void updateScreen() override;
    void initGui() override;
    void onGuiClosed() override;

    static std::string generateUnusedFolderName(ISaveFormat *fmt, const std::string &base);

protected:
    void actionPerformed(GuiButton *button) override;
    void keyTyped(char_t c, int_t key) override;
    void mouseClicked(int_t x, int_t y, int_t button) override;

public:
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
    void selectNextField() override;

protected:
    void updateFolderName();
    void updateButtonText();

    GuiScreen *parentScreen;
    GuiTextField *textboxWorldName;
    GuiTextField *textboxSeed;
    std::string folderName;
    std::string gameMode;
    bool generateStructures;
    bool hardcore;
    bool createClicked;
    bool moreOptions;
    GuiButton *gameModeButton;
    GuiButton *moreWorldOptionsButton;
    GuiButton *generateStructuresButton;
    GuiButton *worldTypeButton;
    GuiButton *worldSizeButton;
    bool limitedWorld;
    std::string gameModeDescriptionLine1;
    std::string gameModeDescriptionLine2;
    std::string seed;
    std::string localizedNewWorldText;
    int_t worldTypeIndex;
};
