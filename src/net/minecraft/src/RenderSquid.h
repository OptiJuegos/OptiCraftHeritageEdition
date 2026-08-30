#ifndef RENDERSQUID_H
#define RENDERSQUID_H

#include "RenderLiving.h"

class EntitySquid;
class ModelBase;

class RenderSquid : public RenderLiving {
public:
    RenderSquid(ModelBase* modelBase, float f);

    void renderSquid(EntitySquid* entitySquid, double d, double d1, double d2, float f, float f1);

    virtual void doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) override;

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;

protected:
    virtual void rotateCorpse(EntityLiving* entityLiving, float f, float f1, float f2) override;
    virtual float getTicksExisted(EntityLiving* entityLiving, float partialTicks) override;

    void rotateSquidCorpse(EntitySquid* entitySquid, float f, float f1, float f2);
};

#endif