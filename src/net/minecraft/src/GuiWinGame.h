#pragma once

#include "GuiScreen.h"

#include <string>
#include <vector>

// net.minecraft.src.GuiWinGame
class GuiWinGame : public GuiScreen
{
public:
    GuiWinGame();

    void updateScreen() override;
    void initGui() override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
    bool doesGuiPauseGame() override;

protected:
    void keyTyped(char_t character, int_t key) override;
    void actionPerformed(GuiButton *button) override;

private:
    void respawnPlayer();
    void drawScrollingBackground(float_t partialTick);
    void loadCredits();
    void appendWrappedLine(const std::string &line, int_t widthPixels);
    static std::string replaceAll(std::string value, const std::string &from, const std::string &to);

    int_t updateCounter;
    std::vector<std::string> lines;
    int_t totalScrollHeight;
    float_t scrollSpeed;
};
