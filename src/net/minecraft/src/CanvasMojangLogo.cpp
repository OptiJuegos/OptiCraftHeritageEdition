#include "CanvasMojangLogo.h"

#include <iostream>
#include <fstream>

#include "platform/RenderAPI.h"
#include "java/BufferedImage.h"
#include "GameResources.h"
#include "Tessellator.h"

CanvasMojangLogo::CanvasMojangLogo()
    : logoTextureId(-1), logoWidth(0), logoHeight(0)
{
    try
    {
        // Java: ImageIO.read(PanelCrashReport.class.getResource("/gui/logo.png"))
        std::ifstream file(GameResources::resolve("/gui/logo.png"), std::ios::binary);
        if (file.is_open())
        {
            BufferedImage img = BufferedImage::ImageIO_read(file);
            logoWidth  = img.getWidth();
            logoHeight = img.getHeight();

            if (logoWidth > 0 && logoHeight > 0)
            {
                renderGenerateTextures(1, &logoTextureId);
                renderBindTexture(logoTextureId);
                renderTextureParameters(false, false, false);
                if (renderTextureBeginUpload(logoTextureId, logoWidth, logoHeight, 0, false, false, false))
                    renderTextureImageRgba(0, logoWidth, logoHeight, img.getRawPixels());
            }
        }
    }
    catch (...)
    {
        // Logo not essential; continue without it
    }
}

void CanvasMojangLogo::render(int_t containerWidth)
{
    if (logoTextureId < 0 || logoWidth == 0)
        return;

    int_t x = containerWidth / 2 - logoWidth / 2;
    int_t y = 32;

    renderEnable(RenderCapability::Texture2D);
    renderBindTexture(logoTextureId);

    // One Tessellator quad on every backend. This used to be a glBegin/glEnd
    // pair outside WII_PLATFORM, and it was the only immediate-mode draw left in
    // the game: nine GL entry points existed for this one logo. The Tessellator
    // path is the same one the rest of the GUI already uses, so the console
    // backends consume it natively instead of through the compatibility layer.
    Tessellator *tessellator = &Tessellator::instance;
    tessellator->startDrawingQuads();
    tessellator->addVertexWithUV(x, y + logoHeight, 0.0, 0.0, 1.0);
    tessellator->addVertexWithUV(x + logoWidth, y + logoHeight, 0.0, 1.0, 1.0);
    tessellator->addVertexWithUV(x + logoWidth, y, 0.0, 1.0, 0.0);
    tessellator->addVertexWithUV(x, y, 0.0, 0.0, 0.0);
    tessellator->draw();
}
