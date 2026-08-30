#include "RenderTNTPrimed.h"
#include "EntityTNTPrimed.h"
#include "Block.h"
#include "RenderBlocks.h"
#include "Tessellator.h"
#include "platform/RenderAPI.h"

RenderTNTPrimed::RenderTNTPrimed() {
    shadowSize = 0.5f;
}

void RenderTNTPrimed::renderTNTPrimed(EntityTNTPrimed* entityTNTPrimed, double d, double d1, double d2, float f, float f1) {
    renderPushMatrix();
    renderTranslate((float)d, (float)d1, (float)d2);
    if (((float)entityTNTPrimed->fuse - f1) + 1.0f < 10.0f) {
        float f2 = 1.0f - (((float)entityTNTPrimed->fuse - f1) + 1.0f) / 10.0f;
        if (f2 < 0.0f) {
            f2 = 0.0f;
        }
        if (f2 > 1.0f) {
            f2 = 1.0f;
        }
        f2 *= f2;
        f2 *= f2;
        float f3 = 1.0f + f2 * 0.3f;
        renderScale(f3, f3, f3);
    }
    float f4 = (1.0f - (((float)entityTNTPrimed->fuse - f1) + 1.0f) / 100.0f) * 0.8f;
    renderBlockOnTNT(entityTNTPrimed->getEntityBrightness(f1));
    if (entityTNTPrimed->fuse / 5 % 2 == 0) {
        renderDisable(RenderCapability::Texture2D);
        renderDisable(RenderCapability::Lighting);
        renderEnable(RenderCapability::Blend);
        renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::DstAlpha);
        renderColor4f(1.0f, 1.0f, 1.0f, f4);
        renderBlockOnTNT(1.0f);
        renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        renderDisable(RenderCapability::Blend);
        renderEnable(RenderCapability::Lighting);
        renderEnable(RenderCapability::Texture2D);
    }
    renderPopMatrix();
}

void RenderTNTPrimed::renderBlockOnTNT(float brightness) {
    loadTexture("/terrain.png");
    renderBlocks->renderBlockAsItem(Block::tnt, 0, brightness);
}

void RenderTNTPrimed::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderTNTPrimed((EntityTNTPrimed*)entity, d, d1, d2, f, f1);
}
