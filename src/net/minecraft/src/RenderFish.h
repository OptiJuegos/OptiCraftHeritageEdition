#ifndef RENDERFISH_H
#define RENDERFISH_H

#include "Render.h"

class EntityFish;

class RenderFish : public Render {
public:
    RenderFish();

    void doRenderFish(EntityFish* entityfish, double d, double d1, double d2, float f, float f1);

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;
};

#endif