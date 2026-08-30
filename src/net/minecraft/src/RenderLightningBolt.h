#ifndef RENDERLIGHTNINGBOLT_H
#define RENDERLIGHTNINGBOLT_H

#include "Render.h"

class EntityLightningBolt;

class RenderLightningBolt : public Render {
public:
    RenderLightningBolt();

    void doRenderLightningBolt(EntityLightningBolt* entitylightningbolt, double d, double d1, double d2, float f, float f1);

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;
};

#endif