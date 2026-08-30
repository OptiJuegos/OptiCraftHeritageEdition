#ifndef RENDERCHICKEN_H
#define RENDERCHICKEN_H

#include "RenderLiving.h"

class EntityChicken;
class ModelBase;

class RenderChicken : public RenderLiving {
public:
    RenderChicken(ModelBase* modelBase, float f);

    void renderChicken(EntityChicken* entityChicken, double d, double d1, double d2, float f, float f1);

protected:
    float getWingRotation(EntityChicken* entityChicken, float f);

    float getTicksExisted(EntityLiving* entityLiving, float f) override;

    virtual void doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) override;

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;
};

#endif
