#include "TileEntitySpecialRenderer.h"
#include "TileEntityRenderer.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "Config.h"
#include "Minecraft.h"

TileEntitySpecialRenderer::TileEntitySpecialRenderer() {
    tileEntityRenderer = nullptr;
}

void TileEntitySpecialRenderer::setTileEntityRenderer(TileEntityRenderer* renderer) {
    tileEntityRenderer = renderer;
}

FontRenderer* TileEntitySpecialRenderer::getFontRenderer() {
    return tileEntityRenderer->fontRenderer;
}

void TileEntitySpecialRenderer::bindTextureByName(const std::string& path) {
    RenderEngine* renderengine =
        tileEntityRenderer != nullptr ? tileEntityRenderer->renderEngine : nullptr;
    if (renderengine == nullptr) {
        // GUI-only entry points (the inventory chest/sign item icons -- and,
        // on the 3DS build this crash was found with, the HUD's chest touch
        // button, whose icon has since become a plain 2D crop) can reach
        // here BEFORE the first world render pass ever ran
        // cacheActiveRenderInfo -- TileEntityRenderer::renderEngine is still
        // null and the plain member dereference was a null-this getTexture()
        // crash on 3DS (Luma FAR 0x08 read, 2026-10-01). Pull the engine
        // from the running game instead, and cache it for the next bind. If
        // no game is running there is nothing to bind; skip rather than
        // crash.
        Minecraft* mc = Config::getMinecraft();
        if (mc == nullptr || mc->renderEngine == nullptr)
            return;
        renderengine = mc->renderEngine;
        if (tileEntityRenderer != nullptr) {
            tileEntityRenderer->renderEngine = renderengine;
            if (tileEntityRenderer->fontRenderer == nullptr)
                tileEntityRenderer->fontRenderer = mc->fontRenderer;
        }
    }
    renderengine->bindTexture(renderengine->getTexture(path));
}
