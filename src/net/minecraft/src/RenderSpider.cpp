#include "RenderSpider.h"
#include "EntitySpider.h"
#include "ModelSpider.h"
#include "EntityLiving.h"
#include "Entity.h"
#include "MathHelper.h"
#include "OpenGlHelper.h"
#include "platform/RenderAPI.h"

RenderSpider::RenderSpider() : RenderLiving(new ModelSpider(), 1.0f) {
    setRenderPassModel(new ModelSpider());
}

RenderSpider::~RenderSpider()
{
    delete renderPassModel;
    renderPassModel = nullptr;
}

void RenderSpider::renderSpider(EntitySpider* entitySpider, double d, double d1, double d2, float f, float f1) {
    RenderLiving::doRenderLiving(entitySpider, d, d1, d2, f, f1);
}

void RenderSpider::doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) {
    renderSpider((EntitySpider*)entityLiving, d, d1, d2, f, f1);
}

void RenderSpider::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderSpider((EntitySpider*)entity, d, d1, d2, f, f1);
}

float RenderSpider::getDeathMaxRotation(EntityLiving* entityLiving) {
    return getSpiderDeathMaxRotation((EntitySpider*)entityLiving);
}

float RenderSpider::getSpiderDeathMaxRotation(EntitySpider* entitySpider) {
    return 180.0f;
}


void RenderSpider::preRenderCallback(EntityLiving *entityLiving, float)
{
    EntitySpider *spider = static_cast<EntitySpider *>(entityLiving);
    float scale = spider->spiderScaleAmount();
    renderScale(scale, scale, scale);
}

int RenderSpider::getColorMultiplier(EntityLiving* entityLiving, float f, float f1) {
    return getSpiderColorMultiplier((EntitySpider*)entityLiving, f, f1);
}

int RenderSpider::getSpiderColorMultiplier(EntitySpider* entitySpider, float f, float f1) {
    return RenderLiving::getColorMultiplier(entitySpider, f, f1);
}


int RenderSpider::setSpiderEyeBrightness(EntitySpider* entitySpider, int i, float)
{
    if (i != 0 || entitySpider == nullptr)
        return -1;
    loadTexture("/mob/spider_eyes.png");
    renderEnable(RenderCapability::Blend);
    renderDisable(RenderCapability::AlphaTest);
    renderBlendFunc(RenderBlendFactor::One, RenderBlendFactor::One);
    OpenGlHelper::setLightmapTextureCoords(OpenGlHelper::lightmapTexUnit, 240.0f, 240.0f);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    return 1;
}

int RenderSpider::shouldRenderPass(EntityLiving* entityLiving, int i, float f)
{
    return setSpiderEyeBrightness(static_cast<EntitySpider*>(entityLiving), i, f);
}
