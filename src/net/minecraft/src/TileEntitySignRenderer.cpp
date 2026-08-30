#include "TileEntitySignRenderer.h"

#include "platform/RenderAPI.h"
#include "TileEntitySign.h"
#include "SignModel.h"
#include "Block.h"
#include "FontRenderer.h"
#include "ModelRenderer.h"

TileEntitySignRenderer::TileEntitySignRenderer()
    : signModel(new SignModel())
{
}

TileEntitySignRenderer::~TileEntitySignRenderer()
{
    delete signModel;
    signModel = nullptr;
}

void TileEntitySignRenderer::renderTileEntitySignAt(TileEntitySign* tileentitysign, double d, double d1, double d2, float f)
{
    Block* block = tileentitysign->getBlockType();
    renderPushMatrix();
    float f1 = 0.6666667f;
    if (block == Block::signPost)
    {
        renderTranslate((float)d + 0.5f, (float)d1 + 0.75f * f1, (float)d2 + 0.5f);
        float f2 = (float)(tileentitysign->getBlockMetadata() * 360) / 16.0f;
        renderRotate(-f2, 0.0f, 1.0f, 0.0f);
        signModel->signStick->showModel = true;
    }
    else
    {
        int i = tileentitysign->getBlockMetadata();
        float f3 = 0.0f;
        if (i == 2) f3 = 180.0f;
        if (i == 4) f3 = 90.0f;
        if (i == 5) f3 = -90.0f;
        renderTranslate((float)d + 0.5f, (float)d1 + 0.75f * f1, (float)d2 + 0.5f);
        renderRotate(-f3, 0.0f, 1.0f, 0.0f);
        renderTranslate(0.0f, -0.3125f, -0.4375f);
        signModel->signStick->showModel = false;
    }
    bindTextureByName("/item/sign.png");
    renderPushMatrix();
    renderScale(f1, -f1, -f1);
    signModel->renderSign();
    renderPopMatrix();
    FontRenderer* fontrenderer = getFontRenderer();
    float f4 = (1.0f / 60.0f) * f1;
    renderTranslate(0.0f, 0.5f * f1, 0.07f * f1);
    renderScale(f4, -f4, f4);
    renderNormal3f(0.0f, 0.0f, -1.0f * f4);
    renderDepthMask(false);
    for (int k = 0; k < 4; k++)
    {
        std::string s = tileentitysign->signText[k];
        if (k == tileentitysign->lineBeingEdited)
            s = "> " + s + " <";
        fontrenderer->drawString(s, -fontrenderer->getStringWidth(s) / 2, k * 10 - 20, 0);
    }
    renderDepthMask(true);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderPopMatrix();
}

void TileEntitySignRenderer::renderTileEntityAt(TileEntity* tileentity, double d, double d1, double d2, float f)
{
    renderTileEntitySignAt((TileEntitySign*)tileentity, d, d1, d2, f);
}
