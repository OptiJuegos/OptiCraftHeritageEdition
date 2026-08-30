#pragma once

#include "java/Type.h"

// net.minecraft.src.CanvasMojangLogo
// Java: extends Canvas, loads /gui/logo.png and paints it centred at y=32.
// C++: loads the logo image; render() draws it via OpenGL at the same
// centred position within a given width.
class CanvasMojangLogo
{
public:
    CanvasMojangLogo();

    // Draw the logo centred horizontally within containerWidth at y = 32.
    void render(int_t containerWidth);

    static constexpr int_t PREFERRED_SIZE = 100;

private:
    int_t logoTextureId;   // OpenGL texture ID, -1 if not loaded
    int_t logoWidth;
    int_t logoHeight;
};
