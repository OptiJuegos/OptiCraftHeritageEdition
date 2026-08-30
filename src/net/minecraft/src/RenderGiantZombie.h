#ifndef RENDERGIANTZOMBIE_H
#define RENDERGIANTZOMBIE_H

#include "RenderLiving.h"

class EntityGiantZombie;
class ModelBase;

class RenderGiantZombie : public RenderLiving {
public:
    RenderGiantZombie(ModelBase* modelbase, float f, float f1);

protected:
    void preRenderScale(EntityGiantZombie* entitygiantzombie, float f);

    virtual void preRenderCallback(EntityLiving* entityliving, float f) override;

private:
    float scale;
};

#endif