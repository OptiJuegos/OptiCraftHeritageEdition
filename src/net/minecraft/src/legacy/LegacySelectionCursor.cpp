#include "LegacySelectionCursor.h"

#include <algorithm>

#include "LegacyUiTexture.h"
#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/RenderEngine.h"
#include "net/minecraft/src/Tessellator.h"
#include "platform/PlatformConfig.h"
#include "platform/PlatformTuning.h"
#include "platform/RenderAPI.h"

namespace
{
LegacyUiTexture g_selectionCursor("/cursor.png");

static void emitQuad(Tessellator *tess, int_t x1, int_t y1, int_t x2, int_t y2, float_t zLevel)
{
    tess->addVertex(x1, y2, zLevel);
    tess->addVertex(x2, y2, zLevel);
    tess->addVertex(x2, y1, zLevel);
    tess->addVertex(x1, y1, zLevel);
}

// [Cursor Enhancement / Issue #3 Fix]:
// Procedural fallback that replicates the classic Minecraft: Legacy Console Edition (Xbox 360 / PS3 / Wii U)
// four-arm crosshair cursor with a central ring and hollow center.
//
// Why this exists:
// Previously, GuiScreen and cursor routines directly bound /cursor.png without validating if the
// asset was present or loaded. When cursor.png was absent, RenderEngine fell back to the 16x16
// checkerboard error texture (missingTextureImage), resulting in an unsightly corrupted box.
// This procedural renderer uses Tessellator quads to draw the authentic cursor vectorially so
// the game NEVER falls back to a checkerboard error pattern, even if the PNG texture is missing.
static void drawLegacyProceduralCursor(int_t centerX, int_t centerY, int_t size, float_t zLevel)
{
    const int_t r = std::max<int_t>(6, size / 2);
    const int_t cx = centerX;
    const int_t cy = centerY;

    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    renderDisable(RenderCapability::Texture2D);

    Tessellator *tess = &Tessellator::instance;

    // 1. Black outer cross and ring outline
    renderColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    tess->startDrawingQuads();
    emitQuad(tess, cx - 2, cy - r, cx + 2, cy + r, zLevel);
    emitQuad(tess, cx - r, cy - 2, cx + r, cy + 2, zLevel);
    emitQuad(tess, cx - 4, cy - 4, cx + 4, cy + 4, zLevel);
    tess->draw();

    // 2. White cross arms and surrounding ring
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    tess->startDrawingQuads();
    emitQuad(tess, cx - 1, cy - r + 1, cx + 1, cy - 3, zLevel);
    emitQuad(tess, cx - 1, cy + 3, cx + 1, cy + r - 1, zLevel);
    emitQuad(tess, cx - r + 1, cy - 1, cx - 3, cy + 1, zLevel);
    emitQuad(tess, cx + 3, cy - 1, cx + r - 1, cy + 1, zLevel);

    emitQuad(tess, cx - 3, cy - 3, cx + 3, cy - 2, zLevel);
    emitQuad(tess, cx - 3, cy + 2, cx + 3, cy + 3, zLevel);
    emitQuad(tess, cx - 3, cy - 2, cx - 2, cy + 2, zLevel);
    emitQuad(tess, cx + 2, cy - 2, cx + 3, cy + 2, zLevel);
    tess->draw();

    // 3. Inner black square surrounding transparent center hole
    renderColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    tess->startDrawingQuads();
    emitQuad(tess, cx - 2, cy - 2, cx + 2, cy - 1, zLevel);
    emitQuad(tess, cx - 2, cy + 1, cx + 2, cy + 2, zLevel);
    emitQuad(tess, cx - 2, cy - 1, cx - 1, cy + 1, zLevel);
    emitQuad(tess, cx + 1, cy - 1, cx + 2, cy + 1, zLevel);
    tess->draw();

    renderEnable(RenderCapability::Texture2D);
    renderDisable(RenderCapability::Blend);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}
}

// Draws the selection cursor centered at (centerX, centerY).
// First attempts to render the authentic textured cursor (/cursor.png) if valid and loaded.
// If the texture cannot be loaded or resolved, seamlessly falls back to the procedural reticle.
void legacyDrawSelectionCursorCentered(Minecraft *mc, int_t centerX, int_t centerY,
    int_t size, float_t zLevel)
{
    if (mc == nullptr || mc->renderEngine == nullptr || size <= 0)
        return;

#if PLATFORM_CURSOR_TEXTURE
    // Resolve texture ID safely; LegacyUiTexture will return -1 if not loaded or failed
    const int_t texture = g_selectionCursor.resolve(mc->renderEngine);
    if (texture >= 0)
    {
        legacyDrawUiTexture(texture, centerX - size / 2, centerY - size / 2,
            size, size, zLevel + 1.0f);
        return;
    }
#endif

    // Procedural fallback when texture is unavailable
    drawLegacyProceduralCursor(centerX, centerY, size, zLevel + 1.0f);
}

void legacyDrawSelectionCursor(Minecraft *mc, int_t controlX, int_t controlY,
    int_t controlHeight, float_t zLevel)
{
    const int_t size = std::max<int_t>(10, std::min<int_t>(PLATFORM_CURSOR_SIZE, controlHeight));
    legacyDrawSelectionCursorCentered(mc, controlX - 4, controlY + controlHeight / 2,
        size, zLevel);
}
