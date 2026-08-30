#ifndef RENDERLIVING_H
#define RENDERLIVING_H

#include "Render.h"

class ModelBase;
class EntityLiving;
class FontRenderer;

class RenderLiving : public Render {
public:
    RenderLiving(ModelBase* modelbase, float f);
    ~RenderLiving() override;

    void setRenderPassModel(ModelBase* modelbase);

    virtual void doRenderLiving(EntityLiving* entityliving, double d, double d1, double d2, float f, float f1);

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;

protected:
    virtual void translateEntity(EntityLiving* entityliving, double d, double d1, double d2);
    virtual void rotateCorpse(EntityLiving* entityliving, float f, float f1, float f2);
    float getSwingProgress(EntityLiving* entityliving, float f);
    virtual float getTicksExisted(EntityLiving* entityliving, float f);
    virtual void renderEquippedItems(EntityLiving* entityliving, float f);
    virtual void renderModel(EntityLiving* entityliving, float f, float f1, float f2, float f3, float f4, float f5);
    virtual int inheritRenderPass(EntityLiving* entityliving, int i, float f);
    virtual int shouldRenderPass(EntityLiving* entityliving, int i, float f);
    virtual float getDeathMaxRotation(EntityLiving* entityliving);
    virtual int getColorMultiplier(EntityLiving* entityliving, float f, float f1);
    virtual void preRenderCallback(EntityLiving* entityliving, float f);
    virtual void passSpecialRender(EntityLiving* entityliving, double d, double d1, double d2);
    void renderLivingLabel(EntityLiving* entityliving, std::string s, double d, double d1, double d2, int i);

    ModelBase* mainModel;
    ModelBase* renderPassModel;
};

#endif
