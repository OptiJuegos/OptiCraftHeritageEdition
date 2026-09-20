#include "LegacyUiAssets.h"

#include "LegacyUiPolicy.h"
#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/RenderEngine.h"
#include "net/minecraft/src/Tessellator.h"
#include "platform/RenderAPI.h"
#include "platform/PlatformConfig.h"

namespace
{
struct LegacyTitleTextureCache
{
    RenderEngine *engine = nullptr;
    int_t texture = -1;
    bool resourceChecked = false;
    bool resourceAvailable = false;
};

LegacyTitleTextureCache g_titleTextureCache;

int_t resolveLegacyTitleTexture(RenderEngine *engine, const char *path)
{
    if (engine == nullptr || path == nullptr)
        return -1;

    if (g_titleTextureCache.engine != engine)
    {
        g_titleTextureCache.engine = engine;
        g_titleTextureCache.texture = -1;
        g_titleTextureCache.resourceChecked = false;
        g_titleTextureCache.resourceAvailable = false;
    }

    if (!g_titleTextureCache.resourceChecked)
    {
        g_titleTextureCache.resourceAvailable = engine->hasResource(path);
        g_titleTextureCache.resourceChecked = true;
    }
    if (!g_titleTextureCache.resourceAvailable)
        return -1;

    if (g_titleTextureCache.texture >= 0 && renderTextureIsValid(g_titleTextureCache.texture))
        return g_titleTextureCache.texture;

    const int_t texture = engine->getTexture(path);
    if (!renderTextureIsValid(texture))
        return -1;

    g_titleTextureCache.texture = texture;
    return texture;
}
}

bool legacyDrawTitleTexture(Minecraft *mc, const LegacyMainMenuLayout &layout, int_t screenWidth,
    float_t zLevel, LegacyUiRect *outRect)
{
    if (outRect != nullptr)
        *outRect = LegacyUiRect{};
    if (mc == nullptr || mc->renderEngine == nullptr)
        return false;

    const char *path = legacyUiTitleResourcePath();
    const int_t texture = resolveLegacyTitleTexture(mc->renderEngine, path);
    if (texture < 0)
        return false;
    int_t textureWidth = 0;
    int_t textureHeight = 0;
    if (!mc->renderEngine->getTextureDimensions(texture, &textureWidth, &textureHeight) ||
        textureWidth <= 0 || textureHeight <= 0)
        return false;

    const LegacyUiRect rect = legacyFitTitleRect(screenWidth, layout.titleY, layout.titleMaxWidth,
        layout.titleMaxHeight,
#if PLATFORM_PS2
        274, 44); // Original atlas contains two stacked halves, not a full banner.
#else
        textureWidth, textureHeight);
#endif
    if (rect.width <= 0 || rect.height <= 0)
        return false;

    renderBindTexture(texture);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);

    Tessellator *tess = &Tessellator::instance;
    tess->startDrawingQuads();
    tess->setColorOpaque_I(0xffffff);
#if PLATFORM_PS2
    const auto drawHalf = [&](double x, double w, double sourceY, double sourceWidth)
    {
        const double u = sourceWidth / textureWidth;
        const double v0 = sourceY / textureHeight;
        const double v1 = (sourceY + 44.0) / textureHeight;
        tess->addVertexWithUV(x, rect.y + rect.height, zLevel, 0.0, v1);
        tess->addVertexWithUV(x + w, rect.y + rect.height, zLevel, u, v1);
        tess->addVertexWithUV(x + w, rect.y, zLevel, u, v0);
        tess->addVertexWithUV(x, rect.y, zLevel, 0.0, v0);
    };
    const double firstWidth = rect.width * 155.0 / 274.0;
    drawHalf(rect.x, firstWidth, 0.0, 155.0);
    drawHalf(rect.x + firstWidth, rect.width - firstWidth, 45.0, 119.0);
#else
    tess->addVertexWithUV(rect.x, rect.y + rect.height, zLevel, 0.0, 1.0);
    tess->addVertexWithUV(rect.x + rect.width, rect.y + rect.height, zLevel, 1.0, 1.0);
    tess->addVertexWithUV(rect.x + rect.width, rect.y, zLevel, 1.0, 0.0);
    tess->addVertexWithUV(rect.x, rect.y, zLevel, 0.0, 0.0);
#endif
    tess->draw();
    renderDisable(RenderCapability::Blend);

    if (outRect != nullptr)
        *outRect = rect;
    return true;
}
