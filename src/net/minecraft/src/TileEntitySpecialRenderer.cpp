#include "TileEntitySpecialRenderer.h"
#include "TileEntityRenderer.h"
#include "FontRenderer.h"
#include "RenderEngine.h"

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
    RenderEngine* renderengine = tileEntityRenderer->renderEngine;
    renderengine->bindTexture(renderengine->getTexture(path));
}
