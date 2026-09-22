#pragma once

#include "net/minecraft/src/GuiScreen.h"
#include <string>

class GuiButton;

class GuiSkinSelector : public GuiScreen
{
public:
    explicit GuiSkinSelector(GuiScreen *parent);
    ~GuiSkinSelector() override = default;

    void initGui() override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
    void updateScreen() override;
    void keyTyped(char_t c, int_t key) override;
    void mouseClicked(int_t mouseX, int_t mouseY, int_t button) override;
    void actionPerformed(GuiButton *button) override;
    bool doesGuiPauseGame() override;

protected:
    bool usesSpecializedMenuNavigation() const override { return true; }
    bool suppressesPlatformPointerInput() const override { return true; }
    void handleSpecializedMenuInput() override;

private:
    void nextSkin();
    void prevSkin();
    void selectAndConfirm();
    void cancelAndReturn();

    void drawBeveledPanel(int_t left, int_t top, int_t right, int_t bottom, int_t fillColor);
    void drawInsetPanel(int_t left, int_t top, int_t right, int_t bottom, int_t fillColor);
    void drawFrontPreview(const std::string &path, float x, float y, float w, float h, float alpha);
    void drawFeetShadow(float centerX, float groundY, float radiusX, float radiusY, float alpha);

    GuiScreen *parentScreen;
    int currentSkinIndex;
    float scrollOffset; // smooth transition offset: -1.0 (moving right) to +1.0 (moving left), dampens to 0.0

    // Stored dimensions for click hit-testing
    int_t dialogLeft;
    int_t dialogTop;
    int_t dialogWidth;
    int_t dialogHeight;
    int_t leftPanelWidth;
    int_t rightPanelX;
    int_t rightPanelWidth;
    int_t carouselCenterX;
    int_t carouselGroundY;
    int_t nameplateY;
    int_t nameplateHeight;

    // Disabled Player 2 button
    GuiButton *buttonPlayer2Skin;

#if defined(PS2_PLATFORM) || defined(PLATFORM_PS2)
    bool ps2ActionReleaseLatch;
    bool stickNavLatched;
    int dpadRepeatTimer;
    int stickRepeatTimer;
#endif
};
