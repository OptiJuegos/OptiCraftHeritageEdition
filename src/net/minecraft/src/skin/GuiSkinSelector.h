#pragma once

#include "platform/PlatformConfig.h"
#include "net/minecraft/src/GuiScreen.h"
#include "SkinManager.h"
#include <string>

class GuiButton;

class GuiSkinSelector : public GuiScreen
{
public:
    explicit GuiSkinSelector(GuiScreen *parent, bool isPlayer2 = false);
    ~GuiSkinSelector() override = default;

    void initGui() override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

    // Draws the 2D front preview of a skin (pre-rendered 16x32 preview, or
    // assembled from the skin sheet). Static because it only needs a render
    // context and a z level: the 3DS dual-screen title screen shows the
    // player's skin on its bottom panel without a live selector instance.
    static void drawSkinFrontPreview(Minecraft *mc, float zLevel, const SkinEntry *skin,
        float x, float y, float w, float h, float alpha);
    void updateScreen() override;
    void keyTyped(char_t c, int_t key) override;
    bool handleJavaUiNavigationKey(int_t key) override;
    void mouseClicked(int_t mouseX, int_t mouseY, int_t button) override;
    void actionPerformed(GuiButton *button) override;
    bool doesGuiPauseGame() override;

protected:
    bool allowsPlatformPointerInput() const override { return true; }
    void handleSpecializedMenuInput() override;

private:
    void nextSkin();
    void prevSkin();
    void switchPack(int newPackIndex);
    void selectAndConfirm();
    void cancelAndReturn();
    void deleteCurrentCustomSkin();

    void drawBeveledPanel(int_t left, int_t top, int_t right, int_t bottom, int_t fillColor);
    void drawInsetPanel(int_t left, int_t top, int_t right, int_t bottom, int_t fillColor);
    void drawFrontPreview(const SkinEntry *skin, float x, float y, float w, float h, float alpha);
    void drawFeetShadow(float centerX, float groundY, float radiusX, float radiusY, float alpha);

    GuiScreen *parentScreen;
    bool isPlayer2Skin;
    bool initializedSelection;
    int currentPackIndex;
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

    // Interactive GUI buttons
    GuiButton *buttonTabDefault;
    GuiButton *buttonTabCustom;
    GuiButton *buttonPlayer2Skin;
    GuiButton *buttonLoadSkins;
    GuiButton *buttonDeleteSkin;

#if PLATFORM_PS2
    bool ps2ActionReleaseLatch;
    bool stickNavLatched;
    int dpadRepeatTimer;
    int stickRepeatTimer;
    unsigned short lastPadHeld;
#endif
};
