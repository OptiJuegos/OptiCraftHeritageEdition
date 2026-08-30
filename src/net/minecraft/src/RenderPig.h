#ifndef RENDERPIG_H
#define RENDERPIG_H

#include "RenderLiving.h"

class EntityPig;
class ModelBase;

class RenderPig : public RenderLiving {
public:
    RenderPig(ModelBase* modelBase, ModelBase* modelBase2, float shadowSize = 0.7f);
    ~RenderPig() override;

    void renderPig(EntityPig* entityPig, double d, double d1, double d2, float f, float f1);

    virtual void doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) override;

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;

protected:
    int shouldRenderPass(EntityLiving* entityLiving, int i, float f) override;
    int renderSaddledPig(EntityPig* entityPig, int i, float f);
};

#endif