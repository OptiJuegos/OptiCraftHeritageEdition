#pragma once

#include "platform/PlatformConfig.h"
#include "GuiScreen.h"
#include <string>

class GuiButton;

// net.minecraft.src.GuiMainMenu
class GuiMainMenu : public GuiScreen
{
public:
    GuiMainMenu();
    ~GuiMainMenu() override;

    void updateScreen() override;
    bool doesGuiPauseGame() override;

protected:
    bool usesSpecializedMenuNavigation() const override;
    void keyTyped(char_t c, int_t key) override;
    void mouseClicked(int_t x, int_t y, int_t button) override;

public:
    void initGui() override;

protected:
    void actionPerformed(GuiButton *button) override;

public:
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
#if PLATFORM_3DS
    // Dual-screen title: drawScreen() paints the banner half on the top LCD
    // itself and opens the bottom-panel pass for the menu half, so
    // EntityRenderer must not wrap it in one (the passes never nest).
    bool managesBottomPanelPass() const override { return true; }
#endif

private:
    void drawPanorama(int_t mouseX, int_t mouseY, float_t partialTick, float_t aspectRatio);
    void rotateAndBlurSkybox(float_t partialTick, bool copyFramebuffer = true);
    void renderSkybox(int_t mouseX, int_t mouseY, float_t partialTick);
    void syncLegacySelection();
    void moveLegacySelection(int_t direction);
    void activateLegacySelection();
    // Panorama + banner + splash half of the title screen, laid out in the
    // current width/height space: the whole screen on every port, the top
    // LCD's pixels on the 3DS (see drawScreen()).
    void drawTitleArt(int_t mouseX, int_t mouseY, float_t partialTick);
    // Footer (version/copyright or the Legacy hint row) plus the button
    // column, again in the current width/height space.
    void drawMenuFooter(bool legacyUi, int_t mouseX, int_t mouseY, float_t partialTick);
#if PLATFORM_3DS
    // Dual-screen split: the menu half (backdrop + options + player card)
    // draws on the bottom panel in its 320x240 canvas.
    void drawTitleBottomHalf(int_t mouseX, int_t mouseY, float_t partialTick);
    void revealBottomMenu();
    // "Press START Button" gate: the options stay hidden until START (ESC),
    // A (RETURN) or a touch on the panel wakes them. Deliberately not reset
    // in initGui() -- coming back from a sub-screen (Options, Play Game)
    // must not put the gate back up; a fresh instance (boot, leaving a
    // world) starts gated again.
    bool bottomMenuRevealed = false;
#endif

    float_t updateCounter;
    std::string splashText;
    GuiButton *multiplayerButton;
    int_t panoramaTimer;
    int_t viewportTexture;
    bool legacyPanoramaAvailable;
    int_t selectedControlIndex;
    int_t hoveredControlIndex;
};
