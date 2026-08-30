#ifndef RENDERSPIDER_H
#define RENDERSPIDER_H

#include "RenderLiving.h"

class EntitySpider;
class ModelBase;

class RenderSpider : public RenderLiving {
public:
    RenderSpider();
    ~RenderSpider() override;

    void renderSpider(EntitySpider* entitySpider, double d, double d1, double d2, float f, float f1);

    virtual void doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) override;

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;

protected:
    virtual float getDeathMaxRotation(EntityLiving* entityLiving) override;

    float getSpiderDeathMaxRotation(EntitySpider* entitySpider);

    virtual int getColorMultiplier(EntityLiving* entityLiving, float f, float f1) override;
    void preRenderCallback(EntityLiving* entityLiving, float f) override;

    int shouldRenderPass(EntityLiving* entityLiving, int i, float f) override;
    int setSpiderEyeBrightness(EntitySpider* entitySpider, int i, float f);

    int getSpiderColorMultiplier(EntitySpider* entitySpider, float f, float f1);
};

#endif