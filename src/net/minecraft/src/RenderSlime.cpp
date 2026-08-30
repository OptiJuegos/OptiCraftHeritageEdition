#include "RenderSlime.h"
#include "EntitySlime.h"
#include "ModelBase.h"
#include "EntityLiving.h"
#include "Entity.h"
#include "MathHelper.h"
#include "platform/RenderAPI.h"

RenderSlime::RenderSlime(ModelBase* modelBase, ModelBase* modelBase2, float f) : RenderLiving(modelBase, f) {
    scaleAmount = modelBase2;
}

RenderSlime::~RenderSlime()
{
    delete scaleAmount;
    scaleAmount = nullptr;
    renderPassModel = nullptr;
}

void RenderSlime::renderSlime(EntitySlime* entitySlime, double d, double d1, double d2, float f, float f1) {
    RenderLiving::doRenderLiving(entitySlime, d, d1, d2, f, f1);
}

void RenderSlime::doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) {
    renderSlime((EntitySlime*)entityLiving, d, d1, d2, f, f1);
}

void RenderSlime::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderSlime((EntitySlime*)entity, d, d1, d2, f, f1);
}

void RenderSlime::preRenderCallback(EntityLiving* entityLiving, float f) {
    scaleSlime((EntitySlime*)entityLiving, f);
}

void RenderSlime::scaleSlime(EntitySlime* entitySlime, float f) {
    float f1 = (float)entitySlime->getSlimeSize();
    float f2 = (entitySlime->field_767_b + (entitySlime->field_768_a - entitySlime->field_767_b) * f) / (f1 * 0.5f + 1.0f);
    // Corrupt/legacy saves and the old uninitialised C++ slime fields can put
    // the denominator at zero. Never let one entity poison the shared modelview.
    float denominator = f2 + 1.0f;
    if (denominator < 0.001f)
        denominator = 0.001f;
    float f3 = 1.0f / denominator;
    renderScale(f3 * f1, 1.0f / f3 * f1, f3 * f1);
}

int RenderSlime::getColorMultiplier(EntityLiving* entityLiving, float f, float f1) {
    return getSlimeColorMultiplier((EntitySlime*)entityLiving, f, f1);
}

int RenderSlime::getSlimeColorMultiplier(EntitySlime* entitySlime, float f, float f1) {
    return RenderLiving::getColorMultiplier(entitySlime, f, f1);
}


int RenderSlime::renderSlimePassModel(EntitySlime* entitySlime, int i, float)
{
    if (i == 0)
    {
        setRenderPassModel(scaleAmount);
        renderEnable(RenderCapability::Normalize);
        renderEnable(RenderCapability::Blend);
        renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
        return 1;
    }
    if (i == 1)
    {
        renderDisable(RenderCapability::Blend);
        renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    return -1;
}

int RenderSlime::shouldRenderPass(EntityLiving* entityLiving, int i, float f)
{
    return renderSlimePassModel(static_cast<EntitySlime*>(entityLiving), i, f);
}
