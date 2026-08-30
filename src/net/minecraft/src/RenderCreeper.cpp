#include "RenderCreeper.h"
#include "java/Arithmetic.h"
#include "ModelCreeper.h"
#include "EntityCreeper.h"
#include "MathHelper.h"
#include "ModelBase.h"
#include "EntityLiving.h"
#include "platform/RenderAPI.h"

RenderCreeper::RenderCreeper() : RenderLiving(new ModelCreeper(), 0.5f) {
    chargedCreeperModel = new ModelCreeper(2.0f);
}

RenderCreeper::~RenderCreeper()
{
    delete chargedCreeperModel;
    chargedCreeperModel = nullptr;
    renderPassModel = nullptr;
}

void RenderCreeper::updateCreeperScale(EntityCreeper* entityCreeper, float f) {
    float f1 = entityCreeper->setCreeperFlashTime(f);
    float f2 = 1.0f + MathHelper::sin(f1 * 100.0f) * f1 * 0.01f;
    if (f1 < 0.0f) {
        f1 = 0.0f;
    }
    if (f1 > 1.0f) {
        f1 = 1.0f;
    }
    f1 *= f1;
    f1 *= f1;
    float f3 = (1.0f + f1 * 0.4f) * f2;
    float f4 = (1.0f + f1 * 0.1f) / f2;
    renderScale(f3, f4, f3);
}

int RenderCreeper::updateCreeperColorMultiplier(EntityCreeper* entityCreeper, float f, float f1) {
    float f2 = entityCreeper->setCreeperFlashTime(f1);
    if ((int)(f2 * 10.0f) % 2 == 0) {
        return 0;
    }
    int i = (int)(f2 * 0.2f * 255.0f);
    if (i < 0) {
        i = 0;
    }
    if (i > 255) {
        i = 255;
    }
    const uint_t c = 0xffu;
    const uint_t c1 = 0xffu;
    const uint_t c2 = 0xffu;
    return JavaArithmetic::intFromBits((static_cast<uint_t>(i) << 24) | (c << 16) | (c1 << 8) | c2);
}

int RenderCreeper::shouldRenderCreeperPass(EntityCreeper* entityCreeper, int i, float f) {
    if (entityCreeper->getPowered()) {
        if (i == 1) {
            float f1 = (float)entityCreeper->ticksExisted + f;
            loadTexture("/armor/power.png");
            renderMatrixMode(RenderMatrixMode::Texture);
            renderLoadIdentity();
            float f2 = f1 * 0.01f;
            float f3 = f1 * 0.01f;
            renderTranslate(f2, f3, 0.0f);
            setRenderPassModel(chargedCreeperModel);
            renderMatrixMode(RenderMatrixMode::ModelView);
            renderEnable(RenderCapability::Blend);
            float f4 = 0.5f;
            renderColor4f(f4, f4, f4, 1.0f);
            renderDisable(RenderCapability::Lighting);
            renderBlendFunc(RenderBlendFactor::One, RenderBlendFactor::One);
            return 1;
        }
        if (i == 2) {
            renderMatrixMode(RenderMatrixMode::Texture);
            renderLoadIdentity();
            renderMatrixMode(RenderMatrixMode::ModelView);
            renderEnable(RenderCapability::Lighting);
            renderDisable(RenderCapability::Blend);
        }
    }
    return -1;
}

int RenderCreeper::inheritCreeperRenderPass(EntityCreeper* entityCreeper, int i, float f) {
    return -1;
}

void RenderCreeper::preRenderCallback(EntityLiving* entityLiving, float f) {
    updateCreeperScale((EntityCreeper*)entityLiving, f);
}

int RenderCreeper::getColorMultiplier(EntityLiving* entityLiving, float f, float f1) {
    return updateCreeperColorMultiplier((EntityCreeper*)entityLiving, f, f1);
}

int RenderCreeper::shouldRenderPass(EntityLiving* entityLiving, int i, float f) {
    return shouldRenderCreeperPass((EntityCreeper*)entityLiving, i, f);
}

int RenderCreeper::inheritRenderPass(EntityLiving* entityLiving, int i, float f) {
    return inheritCreeperRenderPass((EntityCreeper*)entityLiving, i, f);
}