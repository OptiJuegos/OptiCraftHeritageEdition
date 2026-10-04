#pragma once

#include "net/minecraft/src/GuiButton.h"

class LegacyGuiButton : public GuiButton
{
public:
    LegacyGuiButton(int_t id, int_t x, int_t y, int_t width, int_t height, const std::string &text,
        float_t opacity = 1.0f);

    void drawButton(Minecraft *mc, int_t mouseX, int_t mouseY) override;
    void setSelected(bool selectedValue);
    void setKeyboardSelected(bool selectedValue) override;

    // 3DS dual-screen main menu: a small icon from the game's own assets
    // drawn left of a left-aligned label (Legacy Console's list look). No
    // other screen or platform sets one, so the shared centred label stays
    // the default everywhere else.
    void setMenuIcon(const std::string &path, float u0, float v0, float u1, float v1);

    // 3DS dual-screen title: the +30% column resize (GuiMainMenu's
    // initGui); the base class keeps width/height protected.
    void setButtonSize(int_t w, int_t h)
    {
        width = w;
        height = h;
    }

private:
    float_t opacity;
    bool selected;
    // 3DS main-menu icon (see setMenuIcon); an empty path means "no icon".
    std::string menuIconPath;
    float menuIconU0 = 0.0f;
    float menuIconV0 = 0.0f;
    float menuIconU1 = 1.0f;
    float menuIconV1 = 1.0f;
};
