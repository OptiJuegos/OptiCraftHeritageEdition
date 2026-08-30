#ifndef RENDERCOW_H
#define RENDERCOW_H

#include "RenderLiving.h"

class EntityCow;
class ModelBase;

class RenderCow : public RenderLiving {
public:
    RenderCow(ModelBase* modelBase, float f);

    void renderCow(EntityCow* entityCow, double d, double d1, double d2, float f, float f1);

    virtual void doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) override;

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;
};

#endif