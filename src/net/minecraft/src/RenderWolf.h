#ifndef RENDERWOLF_H
#define RENDERWOLF_H

#include "RenderLiving.h"

class EntityWolf;
class ModelBase;

class RenderWolf : public RenderLiving {
public:
    RenderWolf(ModelBase* modelBase, float shadowSize);

    void renderWolf(EntityWolf* entityWolf, double d, double d1, double d2, float f, float f1);

    virtual void doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) override;

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;

protected:
	float getTicksExisted(EntityLiving* entityLiving, float f) override;

    virtual void preRenderCallback(EntityLiving* entityLiving, float f) override;

};

#endif
